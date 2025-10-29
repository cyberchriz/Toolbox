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
    float 	alpha_cutoff;
    int 	illum;
    int     unique_id;  // unique GLOBAL(!) material index

    MaterialTexIDs global_texIDs;   // not used by this shader
};

// Light Struct to handle multiple light types
struct Light {
    vec4 position;	// xyz=position (World Space - STATIC), w=range/attenuation
    vec4 direction; // xyz=direction (World Space), w=type
    vec4 color;	    // rgb=color, a=intensity
    vec4 spot;	    // x=innerConeAngle, y=outerConeAngle, zw=padding
};

// Define storage buffers (using std430)
// Note: Binding 3 is for model matrices (Vertex Shader only)
layout(binding = 0, set = 0, std430) readonly buffer material_buffer { Material materials[];};
layout(binding = 1, set = 0, std430) readonly buffer light_buffer { Light lights[];};
layout(binding = 2, set = 0, std430) readonly buffer texIDs_buffer { MaterialTexIDs texIDs[];};
layout(binding = 4, set = 0) uniform sampler2D tex_samplers[MAX_TEXTURES];

// Push constants for dynamic data (must match the structure defined in the fragment shader and C++).
layout(push_constant) uniform push_constants {
    mat4 	view; // Camera's view transform
    mat4 	projection;
    vec4 	camera_position; // World Space Camera Position
    uint 	material_index;
    uint 	lights_count;
    vec3    ambient_scene_color;
    float   exposure;
};

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

// Simple PBR Fresnel approximation (Schlick's)
vec3 schlick_fresnel(float NdotV, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - NdotV, 0.0, 1.0), 5.0);
}

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

// Helper function for Geometry Smith (Schlick-GGX)
float schlick_geometry_ggx(float NdotV, float roughness) {
    float k = pow(roughness + 1.0, 2.0) / 8.0; // This is the more correct form
    float numerator = NdotV;
    float denominator = NdotV * (1.0 - k) + k;
    return numerator / max(denominator, EPSILON);
}

// 3. Geometric Attenuation Function (G) - Smith's Method
float smith_geometry(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), EPSILON);
    float NdotL = max(dot(N, L), EPSILON);
    float ggx1 = schlick_geometry_ggx(NdotV, roughness);
    float ggx2 = schlick_geometry_ggx(NdotL, roughness);
    return ggx1 * ggx2;
}

// Simple Reinhard tone mapping operator,
// with Simple Gamma correction to sRGB
vec3 tone_map_reinhard(vec3 color) {
    vec3 result_color = color * exposure;           // Apply exposure (lifts dark areas before tone mapping)
    result_color /= (result_color + vec3(1.0));     // Standard Reinhard tone mapping function: L / (L + 1)
    return pow(result_color, vec3(1.0 / 2.2));      // Simple Gamma correction to sRGB
}

// --- MAIN LIGHTING FUNCTION ---

vec3 calculate_light_contribution(
    Light light,	
    vec3 N_world,		                            // World Space Normal (Perturbed)
    vec3 V_world,		                            // World Space View Direction (Fragment to Camera)
    Material material,	
    vec3 final_color_base,
    vec3 final_specular_color,
    float final_roughness,
    float final_occlusion,
    float final_metallic) {
    
    vec3 light_output = vec3(0.0);
    vec3 L_world; // Light direction vector (World Space, v_position -> Light)
    float distance = 1.0;
    float attenuation = 1.0;
    vec3 radiance;
    int light_type = int(light.direction.w);

    // --- DIRECTIONAL LIGHT ---
    if (light_type == LIGHT_TYPE_DIRECTIONAL) {
        L_world = -normalize(light.direction.xyz);  // World Space calculation: L is direction *to* light.
        radiance = light.color.rgb * light.color.w;
        distance = 1e6;                    // Effectively infinite
    }

    // --- POINT OR SPOT LIGHT ---
    else {
        vec3 light_vec = light.position.xyz - v_position; 
        distance = length(light_vec);
        L_world = normalize(light_vec);
        radiance = light.color.rgb * light.color.w;

        // --- ATTENUATION LOGIC ---
        float range = light.position.w;
        if (range > 0.0) {
            attenuation = 1.0 / (distance * distance + 1.0);            // Inverse Square Falloff (Physical)
            float cutoff = max(0.0, 1.0 - pow(distance / range, 4.0));  // Range Cutoff (Smoothly fading to zero at 'range')
            attenuation *= cutoff;
        }	
        else {
            attenuation = 1.0 / (distance * distance + 1.0);            // No range defined, use inverse square falloff without cutoff
        }
        attenuation = clamp(attenuation, 0.0, 1.0);
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
    // N, L, V, H are all in World Space.
    bool use_pbr = material.illum == 4;

    // --- PBR SHADING PATH ---
    if (use_pbr) {
        
        // --- COMMON VECTORS & DOT PRODUCTS ---
        vec3  H_world   = normalize(V_world + L_world); // Halfway vector
        float NdotL     = max(dot(N_world, L_world), EPSILON);
        float NdotV     = max(dot(N_world, V_world), EPSILON);
        float NdotH     = max(dot(N_world, H_world), EPSILON);
        float HdotV     = max(dot(H_world, V_world), EPSILON); // same as HdotL
        
        // F0 (Fresnel at 0 degrees)
        // Dielectrics F0 = 0.04 (4% reflection), Metals F0 = Base Color
        vec3 F0 = mix(vec3(0.04), final_color_base, final_metallic);

        // Cook-Torrance BRDF (SPECULAR)
        float D = distribution_ggx(N_world, H_world, final_roughness);
        float G = smith_geometry(N_world, V_world, L_world, final_roughness);
        vec3 F = schlick_fresnel(HdotV, F0);

        // --- DIFFUSE BRDF TERM ---
        vec3 kD = (vec3(1.0) - F) * (1.0 - final_metallic); // Energy conservation for diffuse
        vec3 diffuse_brdf = kD * final_color_base / PI;

        // SPECULAR BRDF TERM
        vec3 numerator = D * G * F;
        float denominator = 4.0 * NdotV * NdotL;
        vec3 specular_brdf = numerator / max(denominator, EPSILON);

        // --- 6. FINAL SHADING ---
        vec3 light_output = (diffuse_brdf + specular_brdf) * radiance * NdotL;
        light_output *= final_occlusion;
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
        if (phong_shininess < 0.001 && (material.metallic > 0.0 || material.roughness > 0.0)) {
            // 1. Synthesize Specular Color (Ks) from PBR F0/Metallic:
            // Use 4% F0 (dielectric base) and blend to final_color_base for metals.
            phong_specular_color = mix(vec3(0.04), final_color_base, material.metallic);
        
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

    // --- MATERIAL SELECTION ---
    Material material = materials[material_index];

    // --- VIEW DIRECTION (IN WORLD SPACE) ---
    vec3 V_world = normalize(v_view_dir);                           // World Space View Direction (Fragment to Camera)

    // --- BASE COLOR & ALPHA SAMPLING (SRGB) ---
    vec4 base_color_sample = material.base_color;                   // Start with base color from Material SSBO
    int bc_tex_id = texIDs[material_index].base_color_tex_id;
    if (bc_tex_id >= 0) {
        base_color_sample = texture(tex_samplers[bc_tex_id], v_tex_coord.xy);
    }

    // FIX FOR PURE BLACK SHADOWS (Vertex Color Check)
    vec3 vertex_color_mod = v_color.rgb;
    if (dot(v_color.rgb, v_color.rgb) < EPSILON) {
        vertex_color_mod = vec3(1.0);                               // If black, treat as white to allow base color through
    }

    // Convert base color to Linear Space
    vec3 final_color_base = base_color_sample.rgb * vertex_color_mod;
    float final_alpha = base_color_sample.a * material.dissolve;    // Use sampled alpha, mixed with material dissolve

    // Sample separate Alpha Texture (Alpha/Dissolve Mask)
    int alpha_tex_id = texIDs[material_index].alpha_tex_id;
    if (alpha_tex_id >= 0) {
        // Assume alpha is stored in the Red channel (R) of the alpha mask texture
        float alpha_mask_sample = texture(tex_samplers[alpha_tex_id], v_tex_coord.xy).r;
        final_alpha *= alpha_mask_sample;
    }
    final_alpha = clamp(final_alpha, 0.0, 1.0);

    // --- ALPHA CUTOFF / DISCARD ---
    // If alpha is less than the threshold, discard the fragment (masked blending)
    float blend_mode = material.ambient.w;
    if (blend_mode != 2.0f && final_alpha < material.alpha_cutoff) {
        discard;
    }

    // --- 1. NORMAL MAPPING & TBN ---
    // 1. Normalize the interpolated vectors
    vec3 N = normalize(v_normal);
    vec3 T = normalize(v_tangent.xyz);
    
    // 2. Calculate the Bitangent (B) robustly
    vec3 B = normalize(cross(N, T) * v_tangent.w);

    // 3. Construct the Tangent Space Matrix
    mat3 TBN = mat3(T, B, N);
    
    // Get the sampled normal from the normal/bump map texture if available
    int normal_tex_id = texIDs[material_index].normal_tex_id;
    vec3 sampled_normal = vec3(0.0, 0.0, 1.0); // Default to straight up in tangent space
    
    if (normal_tex_id >= 0) {
        sampled_normal = texture(tex_samplers[normal_tex_id], v_tex_coord.xy).rgb;

        // VULKAN FIX: FLIP THE GREEN CHANNEL (Y-AXIS)
        // (comment out the next line if it causes streaking)
        sampled_normal.g = 1.0 - sampled_normal.g; 

        // Convert from [0, 1] texture range to [-1, 1] vector range and normalize
        sampled_normal = normalize(sampled_normal * 2.0 - 1.0);
    }
    
    // Transform tangent space normal to world space normal
    vec3 N_world = normalize(TBN * sampled_normal);
    
    // Handle backfaces (optional, depends on culling)
    if (dot(N_world, V_world) < 0.0) {
        N_world = -N_world;
    }

    // --- SPECULAR SAMPLING (SRGB) ---
    vec3 final_specular_color = material.specular.rgb;
    int specular_tex_id = texIDs[material_index].specular_tex_id;
    if (specular_tex_id >= 0) {
        // Sample the texture array and multiply by the material specular color
        vec3 specular_sample = texture(tex_samplers[specular_tex_id], v_tex_coord.xy).rgb;
        final_specular_color *= specular_sample;
    };

    // --- METALLIC-ROUGHNESS SAMPLING (PBR) ---
    float final_roughness = material.roughness;
    float final_metallic = material.metallic;

    int mr_tex_id = texIDs[material_index].metallic_roughness_tex_id;
    if (mr_tex_id >= 0) {
        // Sample the texture array using the fetched ID
        vec3 metallic_roughness_sample = texture(tex_samplers[mr_tex_id], v_tex_coord.xy).rgb;
        // gLTF convention: Roughness is in the Green channel, Metallic is in the Blue channel
        final_roughness = metallic_roughness_sample.g;
        final_metallic = metallic_roughness_sample.b;
    }

    // Update the material structure with the final, clamped values for the lighting function
    final_roughness = clamp(final_roughness, 0.0, 1.0);
    final_metallic = clamp(final_metallic, 0.0, 1.0);

    // --- AMBIENT OCCLUSION SAMPLING (R-channel) ---
    float occlusion_factor = 1.0; // Default to 1.0 (no occlusion)
    int occl_tex_id = texIDs[material_index].occlusion_tex_id;
    if (occl_tex_id >= 0) {
        // glTF standard: Red (R) is the Occlusion factor.
        occlusion_factor = texture(tex_samplers[occl_tex_id], v_tex_coord.xy).r;
    }
    occlusion_factor = clamp(occlusion_factor, 0.0, 1.0);

    // --- EMISSIVE SAMPLING (SRGB) ---
    vec3 emission_color = material.emission.rgb;              // Start with material's emission color/strength
    int ec_tex_id = texIDs[material_index].emissive_tex_id;
    if (ec_tex_id >= 0) {
        // glTF standard: RGB is the Emissive color.
        vec3 emissive_sample = texture(tex_samplers[ec_tex_id], v_tex_coord.xy).rgb;
        // The final emission is the material's color multiplied by the texture's color
        emission_color *= emissive_sample;
    }
    
    // --- AMBIENT & EMISSION ---
    vec3 material_ambient_color = material.ambient.rgb;

    int ambient_tex_id = texIDs[material_index].ambient_tex_id;
    if (ambient_tex_id >= 0) {
        vec3 ambient_sample = texture(tex_samplers[ambient_tex_id], v_tex_coord.xy).rgb;
        material_ambient_color *= ambient_sample;
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

    // Apply Occlusion only to the ambient light
    vec3 ambient_light = ambient_scene_color * material_ambient_color * final_color_base * occlusion_factor;
    vec3 emission_light = emission_color;
    vec3 total_lighting = ambient_light + emission_light;

    // --- PER-LIGHT CALCULATION LOOP ---
    for (uint i = 0; i < lights_count; ++i) { 
        total_lighting += calculate_light_contribution(
            lights[i], 
            N_world, 
            V_world,  
            material, 
            final_color_base,
            final_specular_color,
            final_roughness,
            occlusion_factor,
            final_metallic
        );
    }

    // --- FINAL COLOR CALCULATION ---
    // Note: for tone mapping uncomment the line below
    //out_color = vec4(tone_map_reinhard(total_lighting), final_alpha); return;
    out_color = vec4(total_lighting, final_alpha);
}