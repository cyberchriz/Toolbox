#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_shader_8bit_storage : require
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : require
#extension GL_EXT_shader_16bit_storage : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : require
#extension GL_EXT_scalar_block_layout : require

#define MAX_TEXTURES 1024

const float PI = 3.14159265359;
const int LIGHT_TYPE_DIRECTIONAL = 0;
const int LIGHT_TYPE_POINT = 1;
const int LIGHT_TYPE_SPOT = 2;
const float EPSILON = 0.0001;
const vec3 F0_DIELECTRIC = vec3(0.044); 

// ====================================================================================================
// Matching C++ structs MaterialTexIDs & SceneMaterialTexIDs;
// indices must refer to the correct textures of the tex_samplers array buffer
struct MaterialTexIDs {
	int	    ambient_tex_id;
	int	    base_color_tex_id;
	int	    specular_tex_id;
	int	    specular_color_tex_id;

	int	    displacement_tex_id;
	int	    alpha_tex_id;
	int	    reflection_tex_id;
	int	    metallic_roughness_tex_id;	// Combined map for metallic and roughness values

	int	    normal_tex_id;				// Texture used for normal mapping
	int	    occlusion_tex_id;			// Texture used for ambient occlusion.
	int	    emissive_tex_id;
	int     clearcoat_tex_id;

	int     clearcoat_roughness_tex_id;
	int     clearcoat_normal_tex_id;
	int     sheen_color_tex_id;
	int     sheen_roughness_tex_id;

	int     transmission_tex_id;
	int     thickness_tex_id;
	int     specular_gloss_diffuse_tex_id;
	int     specular_gloss_tex_id;
};

// ====================================================================================================
// Matching with C++ Material Struct Layout
struct Material {
    vec4 	ambient;            // ambient.w is used for the blend mode, with 0=OPAQUE, 1=MASK, 2=BLEND
    vec4 	specular;
    vec4 	transmittance;
    vec4 	emission;	
    vec4 	base_color;

    float 	shininess;
    float 	ior;
    float 	dissolve;
    float 	roughness;
    		
    float 	metallic;
    float   diffuse_factor;
    float   glossiness_factor;
    float   specular_factor;

    float   clearcoat_factor;
    float 	alpha_cutoff;
    int 	illum;
    int     unique_id;  // unique GLOBAL(!) material index (typically not used by this shader; the material IDs in this shader refer to the local indices of the materials[] SSBO)

    MaterialTexIDs global_texIDs;   // not used by this shader
};

// ====================================================================================================
// Light Struct to handle multiple light types
struct Light {
    vec4 position;	// xyz=position (World Space - STATIC), w=type
    vec4 direction; // xyz=direction (World Space), w=range/attenuation
    vec4 color;	    // rgb=color, a=intensity
    vec4 spot;	    // x=innerConeAngle, y=outerConeAngle, z=visible, w=global_uniqueID
};

// ====================================================================================================
// Define storage buffers (using std430)
// Note: Binding 3 is for model matrices (Vertex Shader only)
layout(binding = 0, set = 0, std430) readonly buffer material_buffer { Material materials[];};
layout(binding = 1, set = 0, std430) readonly buffer light_buffer { Light lights[];};
layout(binding = 2, set = 0, std430) readonly buffer texIDs_buffer { MaterialTexIDs texIDs[];};
layout(binding = 4, set = 0) uniform samplerCube irradiance_map;  // For diffuse
layout(binding = 5, set = 0) uniform samplerCube prefiltered_map; // For specular
layout(binding = 6, set = 0) uniform sampler2D brdf_lut;          // BRDF lookup table
layout(binding = 7, set = 0) uniform sampler2D tex_samplers[MAX_TEXTURES];

// ====================================================================================================
// Push constants for dynamic data (using std430)
layout(push_constant) uniform push_constants {
	mat4 	view; // Camera's view transform
	mat4 	projection;
	uint 	material_index;
	uint 	lights_count;
	uint    prefiltered_mip_levels;
	float   exposure;
    float   contrast;
	float   ibl_intensity;
	vec3    ambient_scene_color;
	vec3 	camera_position; // World Space Camera Position
};

// ====================================================================================================
// Input variables from the vertex shader (All World Space).
layout(location = 0) in vec4 v_color;		
layout(location = 1) in vec4 v_tangent;	            // World Space Tangent (with v_tangent.w as the handedness)
layout(location = 2) in vec3 v_position;            // World Space Position (P_world)
layout(location = 3) in vec3 v_normal;              // World Space Normal (N_world - pre-normal map)
layout(location = 4) in vec3 v_view_dir;            // World Space View Direction (Fragment to Camera)
layout(location = 5) in vec2 v_tex_coord;
layout(location = 6) flat in uint v_material_index; // GLOBAL(!) Material Index (not used by this shader)

// The output color.
layout(location = 0) out vec4 out_color;




// --- UTILITY FUNCTIONS ---

// ====================================================================================================
// Simple PBR Fresnel approximation (Schlick's)
vec3 schlick_fresnel(float NdotV, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - NdotV, 0.0, 1.0), 5.0);
}

// ====================================================================================================
// Normal Distribution Function (D) - Trowbridge-Reitz GGX
float distribution_ggx(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), EPSILON);
    float NdotH2 = NdotH * NdotH;

    float numerator = a2;
    float denominator = (NdotH2 * (a2 - 1.0) + 1.0);
    denominator = PI * denominator * denominator;
    
    // We clamp to avoid division by zero or infinitesimally small values
    return numerator / max(denominator, EPSILON);
}

// ====================================================================================================
// Helper function for Geometry Smith (Schlick-GGX)
float schlick_geometry_ggx(float NdotV, float roughness) {
    float k = pow(roughness + 1.0, 2.0) / 8.0; // This is the more correct form
    float numerator = NdotV;
    float denominator = NdotV * (1.0 - k) + k;
    return numerator / max(denominator, EPSILON);
}

// ====================================================================================================
// Geometric Attenuation Function (G) - Smith's Method
float smith_geometry(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), EPSILON);
    float NdotL = max(dot(N, L), EPSILON);
    float ggx1 = schlick_geometry_ggx(NdotV, roughness);
    float ggx2 = schlick_geometry_ggx(NdotL, roughness);
    return ggx1 * ggx2;
}

// ====================================================================================================
// Clearcoat Normal Distribution Function (D) - Trowbridge-Reitz GGX (simplified version)
float distribution_ggx_clearcoat(float NdotH, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

// ====================================================================================================
// Clearcoat Geometry Factor (G) - Smith's Method (Schlick-GGX V term)
float geometry_smith_clearcoat(float NdotL, float NdotV) {
    // k_cc = 0.25 is common for clearcoat
    float k_cc = 0.25; 
    float Gv = NdotV / (NdotV * (1.0 - k_cc) + k_cc);
    float Gl = NdotL / (NdotL * (1.0 - k_cc) + k_cc);
    return 1.0 / max(Gv * Gl, EPSILON);
}

// ====================================================================================================
// Sheen Distribution Function (D) - Charlie (Approximation for cloth)
float distribution_sheen_charlie(float NdotH, float sheen_roughness) {
    // sheen_roughness is 1 - alpha in the glTF extension spec
    float alpha = 1.0 - sheen_roughness;
    float a2 = alpha * alpha;
    float inv_a2 = 1.0 / a2;
    float D = 2.0 * inv_a2 * exp(-(NdotH * NdotH - 1.0) * inv_a2);
    return D / (2.0 * PI);
}

// ====================================================================================================
// Simple Reinhard tone mapping operator,
// with Simple Gamma correction to sRGB
vec3 tone_map_reinhard(vec3 color) {
    vec3 result_color = color * exposure;           // Apply exposure (lifts dark areas before tone mapping)
    result_color /= (result_color + vec3(1.0));     // Standard Reinhard tone mapping function: L / (L + 1)
    return pow(result_color, vec3(1.0 / 2.2));      // Simple Gamma correction to sRGB
}

// ====================================================================================================
// ACES tonemapping with contrast adjustment
vec3 tone_map_aces(vec3 color, float contrast) {
    // 1. Apply exposure first
    color *= exposure;
    
    // 2. ACES Filmic Tone Mapping (maps input to a near-[0, 1] range)
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    
    color = (color * (a * color + b)) / (color * (c * color + d) + e);
    color = clamp(color, 0.0, 1.0); 

    // 3. Contrast Adjustment (Centered Power Function)
    float p_exponent = 1.0 - contrast * 0.7; 
    p_exponent = clamp(p_exponent, 0.001, 100.0);                       // safe range
    vec3 c_norm = color * 2.0 - 1.0;                                    // normalize color from [0, 1] to [-1, 1]
    vec3 c_power = sign(c_norm) * pow(abs(c_norm), vec3(p_exponent));   // apply power, preserving the sign
    color = c_power * 0.5 + 0.5;                                        // denormalize color back to [0, 1]
    
    // 4. Gamma correction (applies the display transfer function)
    color = pow(color, vec3(1.0 / 2.2));

    // 5. Final Clamp (Safety and Guarantee)
    return clamp(color, 0.0, 1.0);
}

// ====================================================================================================
// --- MAIN LIGHTING FUNCTION ---

vec3 calculate_light_contribution(
    Light light,	
    vec3 N_world,		                            // World Space Normal (Perturbed)
    vec3 V_world,		                            // World Space View Direction (Fragment to Camera)
    Material material,	                            // Only kept for the 'illum' mode flag
    vec3 final_color_base,                          // Base color (albedo) after texture sampling
    vec3 final_specular_color,                      // Specular color after texture sampling
    float final_roughness,                          // Roughness after texture sampling
    float final_occlusion,                          // Occlusion after texture sampling
    float final_metallic,                           // Metallic after texture sampling
    float final_ior,
    float clearcoat_factor,
    float clearcoat_roughness,
    vec3 clearcoat_normal_world,
    vec3 sheen_color,
    float sheen_roughness,
    vec3 F0) {
    
    vec3 light_output = vec3(0.0);
    vec3 L_world; // Light direction vector (World Space, v_position -> Light)
    float distance = 1.0;
    float attenuation = 1.0;
    vec3 radiance;
    int light_type = int(light.position.w);
    float intensity = light.color.w;

    // --- DIRECTIONAL LIGHT ---
    if (light_type == LIGHT_TYPE_DIRECTIONAL) {
        L_world = -normalize(light.direction.xyz);  // World Space calculation: L is direction *to* light.
        radiance = light.color.rgb * intensity;
        distance = 1e6;                             // Effectively infinite
    }

    // --- POINT OR SPOT LIGHT ---
    else {
        vec3 light_vec = light.position.xyz - v_position; 
        distance = length(light_vec);
        L_world = normalize(light_vec);
        radiance = light.color.rgb * intensity;

        // --- ATTENUATION LOGIC ---
        float range = light.direction.w;
        if (range > EPSILON) {
            attenuation = 1.0 / (distance * distance + 1.0);            // Inverse Square Falloff (Physical)
            float cutoff = max(0.0, 1.0 - pow(distance / range, 4.0));  // Range Cutoff (Smoothly fading to zero at 'range')
            attenuation *= cutoff;
        }	
        else {
            attenuation = 1.0 / (distance * distance + 1.0);            // No range defined, use inverse square falloff without cutoff
        }
    }
    radiance *= attenuation;

    // --- SPOT LIGHT CONE CHECK (World Space calculation) ---
    float spot_effect = 1.0;
    if (light_type == LIGHT_TYPE_SPOT) {
        vec3 D_beam = normalize(light.direction.xyz); 
        float cos_theta = dot(-L_world, D_beam);	
        float cos_inner = cos(light.spot.x);
        float cos_outer = cos(light.spot.y);	    

        // Outside the cone
        if (cos_theta <= cos_outer) {
            spot_effect = 0.0;
        }
        // Smooth transition from inner to outer
        else if (cos_theta < cos_inner) {
            spot_effect = smoothstep(cos_outer, cos_inner, cos_theta);
        }
    }
    radiance *= spot_effect;

    // --- SHADING LOGIC (PBR or Phong) ---
    bool use_pbr = material.illum == 4;

    // --- PBR SHADING PATH ---
    if (use_pbr) {

        // --- COMMON VECTORS & DOT PRODUCTS (Base Material) ---
        vec3  H_world   = normalize(V_world + L_world); 
        float NdotL     = max(dot(N_world, L_world), EPSILON);
        float NdotV     = max(dot(N_world, V_world), EPSILON);
        float NdotH     = max(dot(N_world, H_world), EPSILON);
        float HdotV     = max(dot(H_world, V_world), EPSILON);

        float roughness_base = final_roughness;

        // Cook-Torrance BRDF (SPECULAR)
        float D = distribution_ggx(N_world, H_world, final_roughness);
        float G = smith_geometry(N_world, V_world, L_world, final_roughness);
        vec3 F = schlick_fresnel(HdotV, F0);

        // --- DIFFUSE BRDF TERM ---
        vec3 kD = (vec3(1.0) - F) * (1.0 - final_metallic); 
        vec3 diffuse_brdf = kD * final_color_base / PI;

        // SPECULAR BRDF TERM
        vec3 numerator = D * G * F;
        float denominator = 4.0 * NdotV * NdotL;
        vec3 specular_brdf = numerator / max(denominator, EPSILON);

        // --- CORE SHADING RESULT ---
        vec3 L_base = (diffuse_brdf + specular_brdf) * radiance * NdotL;

        // --- SHEEN (KHR_materials_sheen) ---
        vec3 L_sheen = vec3(0.0);
        if (dot(sheen_color, sheen_color) > EPSILON) {
            float D_sheen = distribution_sheen_charlie(NdotH, sheen_roughness);
            float F_sheen = pow(clamp(1.0 - HdotV, 0.0, 1.0), 5.0); 
            L_sheen = (D_sheen * F_sheen / NdotL) * sheen_color * radiance * NdotL;

            // Energy conservation: attenuate base by sheen, then add sheen on top
            float sheen_strength = max(sheen_color.r, max(sheen_color.g, sheen_color.b));
            L_base = L_base * (1.0 - sheen_strength * F_sheen) + L_sheen;
        }

        // --- CLEARCOAT (KHR_materials_clearcoat) ---
        vec3 L_clearcoat = vec3(0.0);
        if (clearcoat_factor > EPSILON) {
            // Clearcoat Normal/Vectors
            vec3 N_cc = clearcoat_normal_world;
            vec3 H_cc = normalize(V_world + L_world); 
            float NdotL_cc = max(dot(N_cc, L_world), EPSILON);
            float NdotV_cc = max(dot(N_cc, V_world), EPSILON);
            float NdotH_cc = max(dot(N_cc, H_cc), EPSILON);
            
            // Clearcoat BRDF (using its own roughness)
            float D_cc = distribution_ggx_clearcoat(NdotH_cc, clearcoat_roughness);
            float G_cc = geometry_smith_clearcoat(NdotL_cc, NdotV_cc);
            vec3 F_cc = schlick_fresnel(NdotV_cc, F0_DIELECTRIC);
            
            // Clearcoat Layer
            vec3 cc_brdf = (D_cc * G_cc * F_cc) / (4.0 * NdotL_cc * NdotV_cc);
            L_clearcoat = cc_brdf * radiance * NdotL_cc * clearcoat_factor;

            // Attenuate base layer by the clearcoat Fresnel, scaled by clearcoat_factor
            vec3 F_cc_base = schlick_fresnel(NdotV_cc, F0_DIELECTRIC);
            L_base = L_base * (vec3(1.0) - F_cc_base * clearcoat_factor) + L_clearcoat;
        }

        // Final occlusion application
        light_output = L_base * final_occlusion;
        return light_output;
    }	
    else {
        // --- LEGACY PHONG SHADING PATH (Fallback) ---

        // --------------------------------------------------------------------------------
        // PBR-to-Phong Conversion Logic
        // Synthesize valid Phong inputs (Ks and Ns) if the material is PBR-sourced 
        // and lacks legacy data (e.g., shininess=0 or specular color is black).
        // This prevents the Phong path from resulting in a black screen.
        // --------------------------------------------------------------------------------
        vec3 phong_specular_color = final_specular_color;
        float phong_shininess = material.shininess;

        // Check if the material is likely PBR (e.g., metallic/roughness values present) 
        // and the legacy shininess is effectively zero (a common sign of missing Phong data).
        if (phong_shininess < 0.001 && (material.metallic > 0.045 || material.roughness > 0.0)) {
            // 1. Synthesize Specular Color (Ks) from PBR F0/Metallic:
            // Use 4.5% F0 (dielectric base) and blend to final_color_base for metals.
            phong_specular_color = mix(F0_DIELECTRIC, final_color_base, material.metallic);
        
            // 2. Synthesize Shininess (Ns) from PBR Roughness (inverse relationship):
            float inverse_roughness = 1.0 - material.roughness;
            phong_shininess = inverse_roughness * inverse_roughness * 1000.0;
        }
        // --------------------------------------------------------------------------------

        // Specular (ks * (R.V)^ns * I_light)
        vec3 reflect_dir = reflect(-L_world, N_world);
        
        // Enforce minimum shininess (1.0) for visual stability.
        float final_shininess = max(1.0, phong_shininess);
        
        float spec_strength = pow(max(dot(V_world, reflect_dir), 0.0), final_shininess);
        
        vec3 specular_light = spec_strength * phong_specular_color * radiance;

        // ENERGY CONSERVATION FIX (Phong): Subtract specular energy from diffuse
        float specular_energy_loss = max(max(phong_specular_color.r, phong_specular_color.g), phong_specular_color.b);
        vec3 energy_conserved_diffuse_color = final_color_base * (1.0 - specular_energy_loss);

        // Diffuse (kd * NdotL * I_light)
        float NdotL = max(dot(N_world, L_world), EPSILON);
        vec3 diffuse_light = NdotL * energy_conserved_diffuse_color * radiance;
        
        light_output = diffuse_light + specular_light;
        return light_output;
    }
}

// --- MAIN ---
void main() {
    // ====================================================================================================
    // --- MATERIAL SELECTION ---
    Material material = materials[material_index];
    int blend_mode = int(material.ambient.w);

    // ====================================================================================================
    // --- VIEW DIRECTION (IN WORLD SPACE) ---
    vec3 V_world = normalize(v_view_dir); // World Space View Direction (Fragment to Camera)

    // ====================================================================================================
    // --- TRANSMISSION SAMPLING ---
    float final_transmission = material.transmittance.a;
    int tx_tex_id = texIDs[material_index].transmission_tex_id;
    if (tx_tex_id >= 0) {
        // glTF standard: Transmission is in the Red (R) channel of the texture
        float transmission_sample = texture(tex_samplers[tx_tex_id], v_tex_coord.xy).r;
        if (material.transmittance.a == 0.0f) {
            final_transmission = transmission_sample;
        }
        else {
            final_transmission *= transmission_sample;
        }
    }
    final_transmission = clamp(final_transmission, 0.0, 1.0);

    // ====================================================================================================
    // --- BASE COLOR SAMPLING (SRGB) ---
    vec4 base_color_sample = material.base_color; // Start with base color from Material SSBO
    int bc_tex_id = texIDs[material_index].base_color_tex_id;
    if (bc_tex_id >= 0) {
        base_color_sample = texture(tex_samplers[bc_tex_id], v_tex_coord.xy);
    }
    vec3 final_color_base = base_color_sample.rgb;

    // ====================================================================================================
    // --- ALPHA SAMPLING ---
    // Sample separate Alpha Texture (Alpha/Dissolve Mask)
    float final_alpha = base_color_sample.a * material.base_color.a;
    int alpha_tex_id = texIDs[material_index].alpha_tex_id;
    if (alpha_tex_id >= 0) {
        // Assume alpha is stored in the Red channel (R) of the alpha mask texture
        float alpha_mask_sample = texture(tex_samplers[alpha_tex_id], v_tex_coord.xy).r;
        
        if (material.dissolve < EPSILON) {
            final_alpha *= alpha_mask_sample;
        }
        else {
            final_alpha *= (material.dissolve * alpha_mask_sample);
        }
    }
    else {
        final_alpha *= material.dissolve;
    }
    final_alpha = clamp(final_alpha, 0.0, 1.0);

    // ====================================================================================================
    // --- NORMAL MAPPING & TBN ---
    vec3 N = normalize(v_normal);
    vec3 T = normalize(v_tangent.xyz);
    T = normalize(T - dot(T, N) * N);
    vec3 B = normalize(cross(N, T) * v_tangent.w); // Calculate the Bitangent (B) robustly
    mat3 TBN = mat3(T, B, N);
    
    // Get the sampled normal from the normal/bump map texture if available
    int normal_tex_id = texIDs[material_index].normal_tex_id;
    vec3 sampled_normal = vec3(0.0, 0.0, 1.0); // Default to straight up in tangent space
    if (normal_tex_id >= 0) {
        sampled_normal = texture(tex_samplers[normal_tex_id], v_tex_coord.xy).rgb;
        sampled_normal.g = 1.0 - sampled_normal.g; // Vulkan fix
        sampled_normal = normalize(sampled_normal * 2.0 - 1.0);
    }
    
    // Transform tangent space normal to world space normal
    vec3 N_world = normalize(TBN * sampled_normal);

    // --- View-Dependent Normal Flip ---
    bool is_opaque = (blend_mode == 0) || (final_transmission < EPSILON) || (final_alpha == 1.0);
    if (is_opaque) {
        if (dot(N_world, V_world) < EPSILON) {
            N_world = -N_world;
        }
    } else {
        if (dot(N_world, N) < 0.0) {
            N_world = -N_world;
        }
    }

    // ====================================================================================================
    // --- SPECULAR SAMPLING (SRGB) ---
    vec3 final_specular_color = material.specular.rgb * material.specular_factor;
    int specular_tex_id = texIDs[material_index].specular_tex_id;
    if (specular_tex_id >= 0) {
        // Sample the texture array and multiply by the material specular color
        vec3 specular_sample = texture(tex_samplers[specular_tex_id], v_tex_coord.xy).rgb;
        if (dot(final_specular_color, final_specular_color) < EPSILON) {
            final_specular_color = specular_sample * material.specular_factor;
        }
        else {
            final_specular_color *= specular_sample;
        }
    };
    
    // ====================================================================================================
    // --- METALLIC-ROUGHNESS SAMPLING (PBR) ---
    float final_roughness = material.roughness;
    float final_metallic = material.metallic;

    int mr_tex_id = texIDs[material_index].metallic_roughness_tex_id;
    if (mr_tex_id >= 0) {
        // Sample the texture array using the fetched ID
        vec3 metallic_roughness_sample = texture(tex_samplers[mr_tex_id], v_tex_coord.xy).rgb;
        // gLTF convention: Roughness is in the Green channel
        if (final_roughness < EPSILON) {
            final_roughness = metallic_roughness_sample.g;
        }
        else {
            final_roughness *= metallic_roughness_sample.g;
        }
        // gLTF convention: Metallic is in the Blue channel
        if (final_metallic < EPSILON) {
            final_metallic = metallic_roughness_sample.b;
        }
        else {
            final_metallic = metallic_roughness_sample.b;
        }
    }

    // specular anti-aliasing
    float normal_len = length(fwidth(N_world));
    final_roughness = max(final_roughness, normal_len * 0.5);
    
    // Update the material structure with the final, clamped values for the lighting function
    final_roughness = clamp(final_roughness, 0.045, 1.0); // Never go below 0.045
    final_metallic = clamp(final_metallic, 0.0, 1.0);

    // ====================================================================================================
    // --- IOR FACTOR (KHR_materials_ior) ---
    float final_ior = material.ior;
    
    // ====================================================================================================
    // --- CLEARCOAT SAMPLING (KHR_materials_clearcoat) ---
    float clearcoat_factor = material.clearcoat_factor;
    float clearcoat_roughness = 0.2;
    vec3 clearcoat_normal_tangent = vec3(0.0, 0.0, 1.0); // Default to straight up in tangent space

    int cc_tex_id = texIDs[material_index].clearcoat_tex_id;
    if (cc_tex_id >= 0) {
        // glTF: Clearcoat factor is R channel
        if (clearcoat_factor < EPSILON) {
            clearcoat_factor = texture(tex_samplers[cc_tex_id], v_tex_coord.xy).r;
        }
        else {
            clearcoat_factor *= texture(tex_samplers[cc_tex_id], v_tex_coord.xy).r;
        }
    }

    int ccr_tex_id = texIDs[material_index].clearcoat_roughness_tex_id;
    if (ccr_tex_id >= 0) {
        // glTF: Clearcoat roughness is G channel
        if (clearcoat_roughness < EPSILON) {
            clearcoat_roughness = texture(tex_samplers[ccr_tex_id], v_tex_coord.xy).g;
        }
        else {
            clearcoat_roughness *= texture(tex_samplers[ccr_tex_id], v_tex_coord.xy).g;
        }
    }

    int ccn_tex_id = texIDs[material_index].clearcoat_normal_tex_id;
    if (ccn_tex_id >= 0) {
        // This is a normal map, requires TBN
        vec3 cc_normal_sample = texture(tex_samplers[ccn_tex_id], v_tex_coord.xy).rgb;
        //cc_normal_sample.g = 1.0 - cc_normal_sample.g; // Vulkan fix
        clearcoat_normal_tangent = normalize(cc_normal_sample * 2.0 - 1.0);
    }
    vec3 clearcoat_normal_world = normalize(TBN * clearcoat_normal_tangent);

    clearcoat_factor = clamp(clearcoat_factor, 0.0, 1.0);
    clearcoat_roughness = clamp(clearcoat_roughness, 0.0, 1.0);
    
    // ====================================================================================================
    // --- SHEEN COLOR SAMPLING ---
    vec3 final_sheen_color = vec3(0.0);
    int sc_tex_id = texIDs[material_index].sheen_color_tex_id;
    if (sc_tex_id >= 0) {
        final_sheen_color = texture(tex_samplers[sc_tex_id], v_tex_coord.xy).rgb;
    }

    // ====================================================================================================
    // --- SHEEN ROUGHNESS SAMPLING ---
    float final_sheen_roughness = 0.6; // use as default if no sheen roughness texture is provided
    int sr_tex_id = texIDs[material_index].sheen_roughness_tex_id;
    if (sr_tex_id >= 0) {
        // glTF KHR_materials_sheen standard specifies G channel for roughness map
        final_sheen_roughness *= texture(tex_samplers[sr_tex_id], v_tex_coord.xy).g; 
    }
    final_sheen_roughness = clamp(final_sheen_roughness, 0.045, 1.0);
    
    // ====================================================================================================
    // --- LEGACY: SPECULAR GLOSSINESS OVERRIDE (HIGHER PRIORITY) ---
    int sgd_tex_id = texIDs[material_index].specular_gloss_diffuse_tex_id;
    if (sgd_tex_id >= 0) {
        vec4 sgd_sampled = texture(tex_samplers[sgd_tex_id], v_tex_coord.xy);
        final_color_base = sgd_sampled.rgb * material.diffuse_factor;
        final_roughness = 1.0 - (sgd_sampled.a * material.glossiness_factor);
         
        // Disable Mutually Exclusive Extensions (Sheen, Clearcoat, etc., must be disabled if SG is active)
        final_sheen_color = vec3(0.0);
        final_sheen_roughness = 1.0;
        clearcoat_factor = 0.0;
        final_metallic = 0.045;
    }

    // ====================================================================================================
    // --- F0 CALCULATION & SG SPECULAR OVERRIDE ---
    vec3 final_F0 = mix(F0_DIELECTRIC, final_color_base, final_metallic);
    int sg_tex_id = texIDs[material_index].specular_gloss_tex_id;
    if (sg_tex_id >= 0) {
        final_F0 = texture(tex_samplers[sg_tex_id], v_tex_coord.xy).rgb * material.specular.rgb * material.specular_factor;
    }

    // ====================================================================================================
    // --- EMISSIVE SAMPLING ---
    vec3 emission_color = material.emission.rgb;
    int ec_tex_id = texIDs[material_index].emissive_tex_id;
    if (ec_tex_id >= 0) {
        emission_color = material.emission.rgb;
        vec3 emissive_sample = texture(tex_samplers[ec_tex_id], v_tex_coord.xy).rgb;
        if (dot(emission_color, emission_color) < EPSILON) {
            emission_color = emissive_sample;
        }
        else {
            emission_color *= emissive_sample;
        }
    }

    // ====================================================================================================
    // --- THICKNESS SAMPLING (KHR_materials_volume) ---
    float final_thickness = 0.05; // = use as default if no thickness texture is provided
    int th_tex_id = texIDs[material_index].thickness_tex_id;
    if (th_tex_id >= 0) {
        // glTF: Thickness is B channel (or R/G)
        if (final_thickness < EPSILON) {
            final_thickness = texture(tex_samplers[th_tex_id], v_tex_coord.xy).b;
        }
        else {
            final_thickness *= texture(tex_samplers[th_tex_id], v_tex_coord.xy).b;
        }
    }
    final_thickness = max(final_thickness, 0.0); // Thickness must be non-negative
    
    // ====================================================================================================
    // --- REFLECTION SAMPLING (KHR_materials_reflection - Custom/Deprecated) ---
    // Assuming reflection texture provides a mask (R channel) for IBL contribution
    float reflection_factor = 0.045; // = use as default if no reflection texture is provided
    int refl_tex_id = texIDs[material_index].reflection_tex_id;
    if (refl_tex_id >= 0) {
        reflection_factor = texture(tex_samplers[refl_tex_id], v_tex_coord.xy).r;
    }

    // ====================================================================================================
    // --- OCCLUSION SAMPLING (R-channel) ---
    float final_occlusion = 1.0; // Default to 1.0 (no occlusion)
    int occl_tex_id = texIDs[material_index].occlusion_tex_id;
    if (occl_tex_id >= 0) {
        // glTF standard: Red (R) is the Occlusion factor.
        final_occlusion = texture(tex_samplers[occl_tex_id], v_tex_coord.xy).r;
    }
    final_occlusion = clamp(final_occlusion, 0.0, 1.0);
    
    // ====================================================================================================
    // --- AMBIENT SAMPLING ---
    vec3 material_ambient_color = material.ambient.rgb;
    int ambient_tex_id = texIDs[material_index].ambient_tex_id;
    if (ambient_tex_id >= 0) {
        vec3 ambient_sample = texture(tex_samplers[ambient_tex_id], v_tex_coord.xy).rgb;
        if (dot(material_ambient_color, material_ambient_color) < EPSILON) {
            material_ambient_color = ambient_sample;
        }
        else {
            material_ambient_color *= ambient_sample;
        }
    }
    
    // FIX FOR PHONG BLACK SCREEN (illum != 4):
    // In many legacy models, material.ambient (Ka) is black, but the model is meant to be lit.
    // If the material is NOT PBR, and Ka is near black, we should fall back to using 
    // the final_color_base (Kd) as the ambient coefficient to ensure visibility.
    bool use_pbr = material.illum == 4;
    if (!use_pbr && dot(material_ambient_color, material_ambient_color) < EPSILON) {
        material_ambient_color = final_color_base;
    }
    else if (dot(material_ambient_color, material_ambient_color) < EPSILON) {
         // Safety: If ambient is still black, use white for scaling, but this should only 
         // happen if both Ka and Kd are black, which would be truly invisible.
         material_ambient_color = vec3(1.0);
    }

    // Ambient light for Phong Legacy
    vec3 ambient_light = vec3(0.0);
    if (!use_pbr) {
        ambient_light = ambient_scene_color * material_ambient_color * final_color_base * final_occlusion;
        
        // attenuate ambient light by transmission factor (only reflected light remains)
        ambient_light *= (1.0 - final_transmission);
    }

    // ====================================================================================================
    // TOTAL LIGHTING CALCULATION
 
    vec3 total_lighting = ambient_light + emission_color;
    
    // --- PER-LIGHT CALCULATION LOOP ---
    for (uint i = 0; i < lights_count; ++i) { 
        vec3 reflected_light = calculate_light_contribution(
            lights[i], 
            N_world, 
            V_world,  
            material, 
            final_color_base,
            final_specular_color,
            final_roughness,
            final_occlusion,
            final_metallic,
            final_ior,
            clearcoat_factor,
            clearcoat_roughness,
            clearcoat_normal_world,
            final_sheen_color,
            final_sheen_roughness,
            final_F0
        );
        
        // attenuate reflected light by transmission factor
        reflected_light *= (1.0 - final_transmission);
        
        total_lighting += reflected_light;
    }

    // ====================================================================================================
    // --- IBL CALCULATION ---
    if (use_pbr) {
        // --- BASE LAYER IBL (Diffuse + Specular) ---
        vec3 k_diffuse = (vec3(1.0) - final_F0) * (1.0 - final_metallic);
        vec3 irradiance = texture(irradiance_map, N_world).rgb;
        vec3 diffuse_ibl = irradiance * final_color_base * k_diffuse;
    
        vec3 R = normalize(reflect(-V_world, N_world));
        float NdotV = clamp(dot(N_world, V_world), 0.0, 1.0);
        uint max_mip_level = max(prefiltered_mip_levels - 1, 0);
        float lod = final_roughness * max_mip_level;
        lod = clamp(lod, 0.0, float(max_mip_level));
        vec3 prefiltered_color = textureLod(prefiltered_map, R, lod).rgb;
        vec2 brdf = texture(brdf_lut, clamp(vec2(NdotV, final_roughness), 0.0, 1.0)).rg;

        vec3 specular_ibl = prefiltered_color * (final_F0 * brdf.x + brdf.y);
        vec3 ibl_base = diffuse_ibl + specular_ibl;
        
        // --- SHEEN IBL CONTRIBUTION ---
        vec3 ibl_sheen = vec3(0.0);
        if (dot(final_sheen_color, final_sheen_color) > EPSILON) {
            // Sheen uses a more diffuse IBL lookup (higher roughness)
            // Charlie sheen is cloth-like, so we use a rough approximation
            float sheen_lod = final_sheen_roughness * max_mip_level;
            sheen_lod = clamp(sheen_lod, 0.0, max_mip_level);
            vec3 sheen_prefiltered = textureLod(prefiltered_map, R, sheen_lod).rgb;
        
            // Sheen Fresnel term
            float F_sheen_ibl = pow(clamp(1.0 - NdotV, 0.0, 1.0), 5.0);
        
            // Sheen contribution (simplified - uses prefiltered environment)
            ibl_sheen = sheen_prefiltered * final_sheen_color * F_sheen_ibl;
        
            // Attenuate base layer by sheen
            float sheen_strength = max(final_sheen_color.r, max(final_sheen_color.g, final_sheen_color.b));
            ibl_base = ibl_base * (1.0 - sheen_strength * F_sheen_ibl) + ibl_sheen;
        }
    
        // --- CLEARCOAT IBL CONTRIBUTION ---
        vec3 ibl_clearcoat = vec3(0.0);
        if (clearcoat_factor > EPSILON) {
            // Clearcoat has its own reflection vector and roughness
            vec3 N_cc = normalize(clearcoat_normal_world);
            vec3 R_cc = reflect(-V_world, N_cc);
            float NdotV_cc = max(dot(N_cc, V_world), 0.0);
        
            // Clearcoat uses its own roughness for IBL lookup
            float cc_lod = clearcoat_roughness * max_mip_level;
            cc_lod = clamp(cc_lod, 0.0, max_mip_level);
            vec3 cc_prefiltered = textureLod(prefiltered_map, R_cc, cc_lod).rgb;
        
            // Clearcoat always has F0 = 0.04 (fixed IOR ~1.5)
            vec3 F0_cc = vec3(0.04);
            vec2 cc_brdf = texture(brdf_lut, vec2(NdotV_cc, clearcoat_roughness)).rg;
            ibl_clearcoat = cc_prefiltered * (F0_cc * cc_brdf.x + cc_brdf.y) * clearcoat_factor;
        
            // Attenuate base layer by clearcoat's Fresnel
            vec3 F_cc_ibl = schlick_fresnel(NdotV_cc, F0_cc);
            ibl_base = ibl_base * (vec3(1.0) - F_cc_ibl * clearcoat_factor) + ibl_clearcoat;
        }
    
        // Apply occlusion and transmission to final IBL
        vec3 ibl_contribution = ibl_base * final_occlusion * (1.0 - final_transmission) * ibl_intensity;
        total_lighting += ibl_contribution;
    }
    
    // ====================================================================================================
    // --- FINAL ALPHA CALCULATION & CUTOFF ---
    if (blend_mode == 0) {                                  // OPAQUE_MODE
        if (final_transmission > EPSILON) {
            // discard transmissive fragments in OPAQUE PASS
            discard;
        }
    }
    else if (blend_mode == 1) {                             // MASK_MODE
        // Apply alpha cutoff for Masked regions
        if (final_alpha < material.alpha_cutoff) {
            discard;
        }
    }
    else if (blend_mode == 2) {                             // BLEND_MODE
        if (final_transmission > EPSILON) {
            final_alpha = 1.0 - final_transmission;
        }
        // If final_transmission is 0.0, final_alpha remains the value calculated 
        // earlier from base_color.a, which is correct for standard alpha blending.
    }
    
    final_alpha = clamp(final_alpha, 0.0, 1.0);

    // ====================================================================================================
    // Final NaN / INF check
    total_lighting = max(total_lighting, vec3(0.0)); // Remove negative values
    if (any(isinf(total_lighting)) || any(isnan(total_lighting))) {
        out_color = vec4(1.0, 0.0, 1.0, 1.0); // Magenta = error
        return;
    }

    // ====================================================================================================
    // --- FINAL COLOR OUTPUT (with tone-mapping and contrast) ---
    out_color = vec4(tone_map_aces(total_lighting, contrast), final_alpha);
}