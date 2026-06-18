#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_shader_8bit_storage : require
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : require
#extension GL_EXT_shader_16bit_storage : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : require
#extension GL_KHR_vulkan_glsl : enable
//#extension GL_EXT_scalar_block_layout : require

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

    // UV map IDs (tex_coord)
	int	    ambient_uv_id;
	int	    base_color_uv_id;
	int	    specular_uv_id;
	int	    specular_color_uv_id;

    int	    displacement_uv_id;
	int	    alpha_uv_id;
	int	    reflection_uv_id;
	int	    metallic_roughness_uv_id;

    int	    normal_uv_id;
	int	    occlusion_uv_id;
	int	    emissive_uv_id;
	int     clearcoat_uv_id;

    int     clearcoat_roughness_uv_id;
	int     clearcoat_normal_uv_id;
	int     sheen_color_uv_id;
	int     sheen_roughness_uv_id;

    int     transmission_uv_id;
	int     thickness_uv_id;
	int     specular_gloss_diffuse_uv_id;
	int     specular_gloss_uv_id;
};

// ====================================================================================================
// Matching with host-side C++ Material Struct Layout (16-byte aligned blocks)
struct Material {
    vec4 	ambient;
    vec4 	specular;
    vec4 	transmittance;          // transmittance.xyz: volume thickness/color
    vec4 	emission;	
    vec4 	base_color;
    vec4 	uv_transform;           // xy = offset, zw = scale
    vec4    attenuation_color;      // for volume / thickness (KHR_materials_volume)

    float   uv_rotation;
    float 	shininess;
    float 	ior;
    float 	dissolve;

    float 	roughness;		
    float 	metallic;
    float   diffuse_factor;
    float   glossiness_factor;

    float   specular_factor;
    float   clearcoat_factor;
    float   clearcoat_roughness;
    float   sheen_factor;

    float   alpha_cutoff;
    float   transmission_factor;
    float   thickness_factor;       // for volume / thickness (KHR_materials_volume)
    float   attenuation_distance;   // ""

    int 	illum;
    int	    alpha_mode; // 0=Opaque, 1=Mask, 2=Blend
    int     unique_id;  // unique GLOBAL(!) material index (typically not used by this shader; the material IDs in this shader refer to the local indices of the materials[] SSBO)
    int     padding0;

    MaterialTexIDs global_texIDs;   // not used by this shader (texIDs local to the scene are used instead)
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
layout(location = 1) in vec4 v_tangent;             // World Space Tangent (with v_tangent.w as the handedness)
layout(location = 2) in vec3 v_position;            // World Space Position (P_world)
layout(location = 3) in vec3 v_normal;              // World Space Normal (N_world - pre-normal map)
layout(location = 4) in vec2 v_tex_coord_0;         // Primary UVs
layout(location = 5) in vec2 v_tex_coord_1;         // Secondary UVs
layout(location = 6) in vec2 v_tex_coord_2;         // ""
layout(location = 7) in vec2 v_tex_coord_3;         // ""
layout(location = 8) flat in uint v_material_index; // GLOBAL(!) Material Index (not used by this shader)
layout(location = 9) in vec3 v_view_dir;            // World Space View Direction (Fragment to Camera)

// The output color.
layout(location = 0) out vec4 out_color;




// --- UTILITY FUNCTIONS ---

// ====================================================================================================
// Simple PBR Fresnel approximation (Schlick's)
vec3 schlick_fresnel(float NdotV, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - NdotV, 0.0, 1.0), 5.0);
}

// ====================================================================================================
// Disney Burley Diffuse for better "non-flat" appearance (compared to simple Lambertian)
float diffuse_burley(float NdotL, float NdotV, float LdotH, float roughness) {
    float fd90 = 0.5 + 2.0 * LdotH * LdotH * roughness;
    float lightScatter = (1.0 + (fd90 - 1.0) * pow(1.0 - NdotL, 5.0));
    float viewScatter  = (1.0 + (fd90 - 1.0) * pow(1.0 - NdotV, 5.0));
    return (lightScatter * viewScatter) / PI;
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
// ACES tonemapping with contrast adjustment
vec3 tone_map_aces(vec3 color, float contrast) {

    // EXPOSURE
    color *= exposure;

    // ACES Filmic Tone Mapping (maps input to a near-[0, 1] range)
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    color = (color * (a * color + b)) / (color * (c * color + d) + e);
    color = clamp(color, 0.0, 1.0);

    // ADAPTIVE CONTRAST LOGIC
    float p = pow(2.0, contrast); 
    vec3 sign_vec = sign(color - 0.5);
    vec3 abs_dist = abs(color * 2.0 - 1.0);
    color = 0.5 + 0.5 * sign_vec * pow(abs_dist, vec3(1.0 / p));
    
    // GAMMA CORRECTION
    color = pow(color, vec3(1.0 / 2.2));

    // FINAL SAFETY CLAMP
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
    float final_clearcoat,
    float clearcoat_roughness,
    vec3 clearcoat_normal_world,
    vec3 sheen_color,
    float sheen_roughness,
    vec3 F0) {
    
    vec3 light_output = vec3(0.0);
    vec3 L_world; // Light direction vector (World Space, v_position -> Light)
    float intensity = light.color.w;
    vec3 radiance = light.color.rgb * intensity;
    int light_type = int(light.position.w);

    // --- DIRECTIONAL LIGHT ---
    if (light_type == LIGHT_TYPE_DIRECTIONAL) {
        L_world = -normalize(light.direction.xyz);  // World Space calculation: L is direction *to* light.
    }

    // --- POINT OR SPOT LIGHT ---
    else {
        float range = light.direction.w;
        if (range >= EPSILON) { // range 0.0: infinite / no fall-off
            // Attenuation Logic: Using 'range' as the reference distance where attenuation = 1.0
            vec3 light_vec = light.position.xyz - v_position;
            float dist = length(light_vec);
            float attenuation = (range * range) / (dist * dist);
            radiance *= attenuation;            
        }
    }

    // --- SPOT LIGHT CONE CHECK (World Space calculation) ---
    if (light_type == LIGHT_TYPE_SPOT) {
        float spot_effect = 1.0;
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
        radiance *= spot_effect;
    }

    // Early exit if no light reaches surface
    if (length(radiance) < EPSILON) return vec3(0.0);

    // --- SHADING LOGIC (PBR or Phong) ---
    bool use_pbr = material.illum == 4;

    // --- PBR SHADING PATH ---
    if (use_pbr) {

        // --- COMMON VECTORS & DOT PRODUCTS (Base Material) ---
        vec3  H_world   = normalize(V_world + L_world); 
        float NdotL = clamp(dot(N_world, L_world), 0.001, 1.0);
        float NdotV = clamp(abs(dot(N_world, V_world)), 0.001, 1.0);
        float NdotH = clamp(dot(N_world, H_world), 0.0, 1.0);
        float VdotH = clamp(dot(V_world, H_world), 0.0, 1.0);

        // SPECULAR BRDF TERM (Cook-Torrance)
        float D = distribution_ggx(N_world, H_world, final_roughness);
        float G = smith_geometry(N_world, V_world, L_world, final_roughness);
        vec3 F = schlick_fresnel(VdotH, F0);

        vec3 spec_numerator = D * G * F;
        float spec_denominator = 4.0 * NdotV * NdotL;
        vec3 specular_brdf = spec_numerator / max(spec_denominator, 0.001);

        // DIFFUSE BRDF TERM (Disney Burley)
        float fd = diffuse_burley(NdotL, NdotV, VdotH, final_roughness);
        vec3  kD = (vec3(1.0) - F) * (1.0 - final_metallic); 
        vec3  diffuse_brdf = kD * final_color_base * (fd / PI);

        // Multi-scattering energy compensation
        float reflectivity = max(max(F0.r, F0.g), F0.b);
        float energy_compensation = 1.0 + final_roughness * (1.0 / NdotV - 1.0) * reflectivity;
        specular_brdf *= energy_compensation;

        // --- CORE LIGHTING RESULT ---
        vec3 L_base = (diffuse_brdf + specular_brdf) * radiance * NdotL;

        // --- SHEEN (KHR_materials_sheen) ---
        vec3 L_sheen = vec3(0.0);
        if (max(sheen_color.r, max(sheen_color.g, sheen_color.b)) > EPSILON) {
            float D_sheen = distribution_sheen_charlie(NdotH, sheen_roughness);
            float F_sheen = pow(clamp(1.0 - VdotH, 0.0, 1.0), 5.0); 
            L_sheen = (D_sheen * F_sheen / NdotL) * sheen_color * radiance * NdotL;

            // Energy conservation: attenuate base by sheen, then add sheen on top
            float sheen_strength = max(sheen_color.r, max(sheen_color.g, sheen_color.b));
            L_base = L_base * (1.0 - sheen_strength * F_sheen) + L_sheen;
        }

        // --- CLEARCOAT (KHR_materials_clearcoat) ---
        vec3 L_clearcoat = vec3(0.0);
        if (final_clearcoat > EPSILON) {
            // Clearcoat Normal/Vectors
            vec3 N_cc = clearcoat_normal_world;
            vec3 H_cc = normalize(V_world + L_world); 
            float NdotL_cc = max(dot(N_cc, L_world), EPSILON);
            float NdotV_cc = max(dot(N_cc, V_world), EPSILON);
            float NdotH_cc = max(dot(N_cc, H_cc), EPSILON);
            float VdotH_cc = max(dot(V_world, H_cc), EPSILON);
            
            // Clearcoat BRDF (using its own roughness)
            float D_cc = distribution_ggx_clearcoat(NdotH_cc, clearcoat_roughness);
            float G_cc = geometry_smith_clearcoat(NdotL_cc, NdotV_cc);
            vec3 F_cc = schlick_fresnel(VdotH_cc, F0_DIELECTRIC);
            
            // Clearcoat Layer
            vec3 cc_brdf = (D_cc * G_cc * F_cc) / (4.0 * NdotL_cc * NdotV_cc + EPSILON);
            L_clearcoat = cc_brdf * radiance * NdotL_cc * final_clearcoat;

            // Attenuate base layer by clearcoat Fresnel
            vec3 transmission_tint = (vec3(1.0) - F_cc * final_clearcoat);
            L_base = L_base * transmission_tint + L_clearcoat;
        }

        // Final occlusion application
        return L_base * final_occlusion;
    }	
    else { // --- PHONG SHADING PATH (Legacy Fallback) ---

        float NdotL = max(dot(N_world, L_world), 0.0);
        float NdotV = max(dot(N_world, V_world), 0.0);
        vec3 effective_F0 = mix(F0_DIELECTRIC, final_color_base, final_metallic);
        vec3 F = schlick_fresnel(NdotV, effective_F0);

        // === SHININESS TERM ===
        float phong_shininess;
        if (material.shininess > EPSILON) {
            phong_shininess = (material.shininess + 8.0) / (8.0 * PI);          // Energy conservation
        }
        else { // Fallback: derive from roughness
            float inverse_roughness = 1.0 - final_roughness;
            phong_shininess = inverse_roughness * inverse_roughness * 256.0; 
        }
        phong_shininess = max(1.0, phong_shininess);
        
        // === DIFFUSE TERM ===
        vec3 kD = (vec3(1.0) - F) * (1.0 - final_metallic);                     // Energy conservation
        vec3 diffuse_light = kD * (NdotL / PI) * final_color_base * radiance;   // Normalized (Lambertian)
        
        // === SPECULAR TERM ===
        vec3 phong_specular_color = final_specular_color;
        if (dot(phong_specular_color, phong_specular_color) <= EPSILON) {
            phong_specular_color = mix(F0_DIELECTRIC, final_color_base, final_metallic);
        }
        float normalization_spec = (phong_shininess + 2.0) / (2.0 * PI);        // Energy Normalization
        vec3 reflect_dir = reflect(-L_world, N_world);
        float spec_dot = max(dot(V_world, reflect_dir), 0.0);
        vec3 specular_light = phong_specular_color * F * pow(spec_dot, phong_shininess) * normalization_spec * radiance * NdotL;
        
        return (diffuse_light + specular_light) * final_occlusion;
    }
}

// --- MAIN ---
void main() {
    // ====================================================================================================
    // --- MATERIAL SELECTION ---
    Material material = materials[material_index];
    bool use_pbr = material.illum == 4;

    // ====================================================================================================
    // --- VIEW DIRECTION (IN WORLD SPACE) ---
    vec3 V_world = normalize(v_view_dir); // World Space View Direction (Fragment to Camera)

    // ====================================================================================================
    // --- UV TRANSFORM (KHR_texture_transform) ---
    // Apply offset, scale, and rotation to the input UVs
    float s = sin(material.uv_rotation);
    float c = cos(material.uv_rotation);
    mat2 rot_mat = mat2(c, s, -s, c);
    vec2 uv_offset = material.uv_transform.xy;
    vec2 uv_scale = material.uv_transform.zw;

    vec2 uv[4];
    uv[0] = (rot_mat * (v_tex_coord_0.xy * uv_scale)) + uv_offset;
    uv[1] = v_tex_coord_1.xy;  // Secondary UVs usually untransformed (standard for AO/Lightmaps in glTF)
    uv[2] = v_tex_coord_2.xy;  // ""
    uv[3] = v_tex_coord_3.xy;  // "
    
    // ====================================================================================================
    // --- NORMAL MAPPING & TBN ---

    vec3 N, T, B;

    // 1. Sanitize and normalize geometric normal
    if (any(isnan(v_normal)) || length(v_normal) < 1e-4) {
        N = vec3(0.0, 1.0, 0.0);
    } else {
        N = normalize(v_normal);
    }
    if (!gl_FrontFacing) {N = -N;} // View-dependent normal flip

    // 2. Check if tangent data is usable
    bool tangent_valid =
        !any(isnan(v_tangent.xyz)) &&
        length(v_tangent.xyz) > 0.01 &&
        abs(dot(normalize(v_tangent.xyz), N)) < 0.99;

    // 3. Use mesh tangent if valid
    if (tangent_valid) {
        vec3 T0 = normalize(v_tangent.xyz);

        // Gram-Schmidt orthogonalization
        T = normalize(T0 - N * dot(N, T0));

        // Reconstruct bitangent using handedness
        float handedness = (abs(v_tangent.w) < 0.5) ? 1.0 : sign(v_tangent.w);
        B = normalize(cross(N, T)) * handedness;
    }
    // 4. Fallback: screen-space derivative TBN
    else {
        vec3 dpdx = dFdx(v_position);
        vec3 dpdy = dFdy(v_position);
        vec2 dtdx = dFdx(uv[0]);
        vec2 dtdy = dFdy(uv[0]);

        vec3 T_ = dpdx * dtdy.y - dpdy * dtdx.y;
        vec3 B_ = dpdy * dtdx.x - dpdx * dtdy.x;

        // Normalize and orthogonalize
        T = normalize(T_ - N * dot(N, T_));
        B = normalize(cross(N, T));
    }

    mat3 TBN = mat3(T, B, N);
    
    // Get the sampled normal from the normal/bump map texture if available
    vec3 N_world = N; // default normal
    int normal_tex_id = texIDs[material_index].normal_tex_id;
    if (normal_tex_id >= 0) {
        // the sampled normal OVERRIDES the default (no multiplication here)
        vec3 sampled_normal = texture(tex_samplers[normal_tex_id], uv[texIDs[material_index].normal_uv_id]).rgb;
        sampled_normal.g = 1.0 - sampled_normal.g; // Vulkan Fix (invert green channel)
        N_world = normalize(TBN * normalize(sampled_normal * 2.0 - 1.0));
    }

    // ====================================================================================================
    // --- BASE COLOR SAMPLING (SRGB) ---
    vec4 base_color_sample = material.base_color; // Start with base color from Material SSBO
    int bc_tex_id = texIDs[material_index].base_color_tex_id;
    if (bc_tex_id >= 0) {
        base_color_sample *= texture(tex_samplers[bc_tex_id], uv[texIDs[material_index].base_color_uv_id]);
    }
    vec3 final_color_base = base_color_sample.rgb;

    // ====================================================================================================
    // --- ALPHA SAMPLING ---
    // Sample separate Alpha Texture (Alpha/Dissolve Mask)
    float final_alpha = base_color_sample.a * material.dissolve; // Note: material.dissolve is MTL legacy; should be 1.0 by default if not assigned explicitly
    int alpha_tex_id = texIDs[material_index].alpha_tex_id;
    if (alpha_tex_id >= 0) {
        // Assume alpha is stored in the Red channel (R) of the alpha mask texture
        final_alpha *= texture(tex_samplers[alpha_tex_id], uv[texIDs[material_index].alpha_uv_id]).r;
    }
    final_alpha = clamp(final_alpha, 0.0, 1.0);

    // ====================================================================================================
    // --- TRANSMISSION SAMPLING ---
    float final_transmission = material.transmission_factor;
    int tx_tex_id = texIDs[material_index].transmission_tex_id;
    if (tx_tex_id >= 0) {
        // glTF standard: Transmission is in the Red (R) channel of the texture
        final_transmission *= texture(tex_samplers[tx_tex_id], uv[texIDs[material_index].transmission_uv_id]).r;
    }
    final_transmission = clamp(final_transmission, 0.0, 1.0);

    // Safeguard for materials which encode transparency only as transmission: override final_alpha
    if (final_alpha > (1.0 - final_transmission)) {
        final_alpha = 1.0 - final_transmission;
    }
    // ====================================================================================================
    // --- SPECULAR SAMPLING (SRGB) ---
    vec3 final_specular_color = material.specular.rgb * material.specular_factor;
    int specular_tex_id = texIDs[material_index].specular_tex_id;
    if (specular_tex_id >= 0) {
        // Sample the texture array and multiply by the material specular color
        final_specular_color *= texture(tex_samplers[specular_tex_id], uv[texIDs[material_index].specular_color_uv_id]).rgb;
    };
    
    // ====================================================================================================
    // --- METALLIC-ROUGHNESS SAMPLING (PBR) ---
    float final_roughness = material.roughness;
    float final_metallic = material.metallic;

    int mr_tex_id = texIDs[material_index].metallic_roughness_tex_id;
    if (mr_tex_id >= 0) {
        // Sample the texture array using the fetched ID
        vec3 mr_sample = texture(tex_samplers[mr_tex_id], uv[texIDs[material_index].metallic_roughness_uv_id]).rgb;
        // Safeguard: check if the sample is non-zero to avoid "killing" the material factors
        if (dot(mr_sample.rgb, mr_sample.rgb) > EPSILON) {
            // gLTF convention: Roughness is in the Green channel
            final_roughness *= mr_sample.g;
            // gLTF convention: Metallic is in the Blue channel
            final_metallic *= mr_sample.b;
        }
    }

    // specular anti-aliasing
    float normal_len = length(fwidth(N_world));
    final_roughness = max(final_roughness, normal_len * 0.5);
    
    // Update the material structure with the final, clamped values for the lighting function
    final_roughness = clamp(final_roughness, 0.045, 1.0);
    final_metallic = clamp(final_metallic, 0.0, 1.0);

    // ====================================================================================================
    // --- IOR FACTOR (KHR_materials_ior) ---
    float final_ior = material.ior;
    
    // ====================================================================================================
    // --- CLEARCOAT SAMPLING (KHR_materials_clearcoat) ---
    float final_clearcoat = material.clearcoat_factor;
    float clearcoat_roughness = material.clearcoat_roughness;
    vec3 clearcoat_normal_tangent = vec3(0.0, 1.0, 0.0); // Default to straight up in tangent space

    int cc_tex_id = texIDs[material_index].clearcoat_tex_id;
    if (cc_tex_id >= 0) {
        // glTF: Clearcoat factor is R channel
        final_clearcoat *= texture(tex_samplers[cc_tex_id], uv[texIDs[material_index].clearcoat_uv_id]).r;
    }
    final_clearcoat = clamp(final_clearcoat, 0.0, 1.0);

    int ccr_tex_id = texIDs[material_index].clearcoat_roughness_tex_id;
    if (ccr_tex_id >= 0) {
        // glTF: Clearcoat roughness is G channel
        clearcoat_roughness *= texture(tex_samplers[ccr_tex_id], uv[texIDs[material_index].clearcoat_roughness_uv_id]).g;
    }
    clearcoat_roughness = clamp(clearcoat_roughness, 0.0, 1.0);

    vec3 clearcoat_normal_world = N_world;
    int ccn_tex_id = texIDs[material_index].clearcoat_normal_tex_id;
    if (ccn_tex_id >= 0) {
        // the sampled normal OVERRIDES the default clearcoat normal (no multiplying)
        vec3 ccn_sample = texture(tex_samplers[ccn_tex_id], uv[texIDs[material_index].clearcoat_normal_uv_id]).rgb;
        clearcoat_normal_world = normalize(TBN * normalize(ccn_sample * 2.0 - 1.0));
    }

    // ====================================================================================================
    // --- SHEEN COLOR SAMPLING ---
    vec3 final_sheen_color = vec3(material.sheen_factor);
    int sc_tex_id = texIDs[material_index].sheen_color_tex_id;
    if (sc_tex_id >= 0) {
        final_sheen_color *= texture(tex_samplers[sc_tex_id], uv[texIDs[material_index].sheen_color_uv_id]).rgb;
    }

    // ====================================================================================================
    // --- SHEEN ROUGHNESS SAMPLING ---
    float final_sheen_roughness = 0.6; // use as default if no sheen roughness texture is provided
    int sr_tex_id = texIDs[material_index].sheen_roughness_tex_id;
    if (sr_tex_id >= 0) {
        // glTF KHR_materials_sheen standard specifies G channel for roughness map
        final_sheen_roughness *= texture(tex_samplers[sr_tex_id], uv[texIDs[material_index].sheen_roughness_uv_id]).g; 
    }
    final_sheen_roughness = clamp(final_sheen_roughness, 0.045, 1.0);
    
    // ====================================================================================================
    // --- LEGACY: SPECULAR GLOSSINESS OVERRIDE (HIGHER PRIORITY) ---
    int sgd_tex_id = texIDs[material_index].specular_gloss_diffuse_tex_id;
    if (sgd_tex_id >= 0) {
        vec4 sgd_sampled = texture(tex_samplers[sgd_tex_id], uv[texIDs[material_index].specular_gloss_diffuse_uv_id]);
        final_color_base *= sgd_sampled.rgb * material.diffuse_factor;
        final_roughness = 1.0 - (sgd_sampled.a * material.glossiness_factor);
         
        // Disable Mutually Exclusive Extensions (Sheen, Clearcoat, etc., must be disabled if SG is active)
        final_sheen_color = vec3(0.0);
        final_sheen_roughness = 1.0;
        final_clearcoat = 0.0;
        final_metallic = 0.045;
    }

    // ====================================================================================================
    // --- F0 CALCULATION & SG SPECULAR OVERRIDE ---
    vec3 final_F0 = mix(F0_DIELECTRIC, final_color_base, final_metallic);
    int sg_tex_id = texIDs[material_index].specular_gloss_tex_id;
    if (sg_tex_id >= 0) {
        final_F0 = texture(tex_samplers[sg_tex_id], uv[texIDs[material_index].specular_gloss_uv_id]).rgb * material.specular.rgb * material.specular_factor;
    }

    // ====================================================================================================
    // --- EMISSIVE SAMPLING ---
    vec3 emission_color = material.emission.rgb;
    int ec_tex_id = texIDs[material_index].emissive_tex_id;
    if (ec_tex_id >= 0) {
        emission_color *= texture(tex_samplers[ec_tex_id], uv[texIDs[material_index].emissive_uv_id]).rgb;
    }

    // ====================================================================================================
    // --- THICKNESS SAMPLING (KHR_materials_volume) ---
    float final_thickness = material.thickness_factor;
    int th_tex_id = texIDs[material_index].thickness_tex_id;
    if (th_tex_id >= 0) {
        // Note: glTF spec specifies the GREEN channel for thickness
        final_thickness *= texture(tex_samplers[th_tex_id], uv[texIDs[material_index].thickness_uv_id]).g;
    }
    final_thickness = max(final_thickness, 0.0); // Thickness must be non-negative
    
    // define the color of the volume's interior
    vec3 final_transmittance = vec3(material.transmittance);
    if (material.attenuation_distance > 0.0) {
        // calculate the absorption coefficient (sigma); we use -log(color) to find the rate of decay per unit of distance
        vec3 sigma = -log(material.attenuation_color.rgb) / material.attenuation_distance;
        // Transmittance = e^(-sigma * distance)
        final_transmittance = exp(-sigma * final_thickness);
    }

    // ====================================================================================================
    // --- REFLECTION SAMPLING (KHR_materials_reflection - Custom/Deprecated) ---
    // Assuming reflection texture provides a mask (R channel) for IBL contribution
    float reflection_factor = 0.045; // = use as default if no reflection texture is provided
    int refl_tex_id = texIDs[material_index].reflection_tex_id;
    if (refl_tex_id >= 0) {
        reflection_factor = texture(tex_samplers[refl_tex_id], uv[texIDs[material_index].reflection_uv_id]).r;
    }

    // ====================================================================================================
    // --- OCCLUSION SAMPLING (R-channel) ---
    float final_occlusion = 1.0; // Default to 1.0 (no occlusion)
    int occl_tex_id = texIDs[material_index].occlusion_tex_id;
    if (occl_tex_id >= 0) {
        // glTF standard: Red (R) is the Occlusion factor.
        final_occlusion = texture(tex_samplers[occl_tex_id], uv[texIDs[material_index].occlusion_uv_id]).r;
    }
    final_occlusion = clamp(final_occlusion, 0.0, 1.0);
    
    // ====================================================================================================
    // --- AMBIENT SAMPLING (Ka) ---
    vec3 material_ambient_color = material.ambient.rgb;
    int ambient_tex_id = texIDs[material_index].ambient_tex_id;
    if (ambient_tex_id >= 0) {
        material_ambient_color *= texture(tex_samplers[ambient_tex_id], uv[texIDs[material_index].ambient_uv_id]).rgb;
    }
    
    // Fallback: If Ka is black on legacy models, use the base albedo (Kd)
    if (!use_pbr && dot(material_ambient_color, material_ambient_color) < EPSILON) {
        material_ambient_color = final_color_base;
    }
 
    vec3 final_ambient = ambient_scene_color * material_ambient_color * final_occlusion;
    final_ambient *= (1.0 - final_transmission); // attenuation by transmission

    // ====================================================================================================
    // TOTAL LIGHTING CALCULATION
 
    vec3 total_lighting = final_ambient + emission_color;
    
    // --- PER-LIGHT CALCULATION LOOP ---
    for (uint i = 0; i < lights_count; ++i) { 
        vec3 reflected_light = calculate_light_contribution(
            lights[i], 
            N_world, 
            V_world,  
            material, 
            final_color_base  * (1.0 - final_transmission), // base color attenuated by transmission
            final_specular_color,
            final_roughness,
            final_occlusion,
            final_metallic,
            final_ior,
            final_clearcoat,
            clearcoat_roughness,
            clearcoat_normal_world,
            final_sheen_color,
            final_sheen_roughness,
            final_F0
        );
        total_lighting += reflected_light;
    }


    // ====================================================================================================
    // --- IBL CALCULATION ---
    if (use_pbr && ibl_intensity > EPSILON) {

        float NdotV = clamp(dot(N_world, V_world), 0.0, 1.0);
        vec2 brdf = texture(brdf_lut, clamp(vec2(NdotV, final_roughness), 0.0, 1.0)).rg;

        // --- REFLECTION (SPECULAR IBL) ---
        vec3 R = normalize(reflect(-V_world, N_world));
        float max_mip_level = max(prefiltered_mip_levels - 1, 0);
        float lod = final_roughness * max_mip_level;
        lod = clamp(lod, 0.0, float(max_mip_level));
        vec3 prefiltered_color = textureLod(prefiltered_map, R, lod).rgb;

        // Multi-scattering compensation
        vec3 ms_compensation = vec3(1.0) + final_F0 * (1.0 / max(brdf.x + brdf.y, 0.01) - 1.0);
        vec3 specular_ibl = prefiltered_color * (final_F0 * brdf.x + brdf.y) * ms_compensation;

        // Horizon fading to prevent light leaking from under the surface
        float horizon = clamp(1.0 + dot(R, v_normal), 0.0, 1.0);
        specular_ibl *= horizon * horizon;

        // --- DIFFUSE IBL (ONLY RELEVANT FOR OPAQUE PARTS) ---
        vec3 k_diffuse = (vec3(1.0) - final_F0) * (1.0 - final_metallic);
        vec3 irradiance = texture(irradiance_map, N_world).rgb;
        vec3 diffuse_ibl = irradiance * final_color_base * k_diffuse * (1.0 - final_transmission); // diffuse disappears as transmission increases
    
        // TRANSMISSION / REFRACTION IBL ---
        vec3 transmission_ibl = vec3(0.0);
        if (final_transmission > EPSILON) {
            // Calculate the Refraction Vector (Snell's Law); eta is the ratio of IORs (Air IOR / Material IOR)
            float air_ior = 1.0;
            float material_ior = max(final_ior, 1.0001); // Prevent div by zero
            float eta = air_ior / material_ior;

            // Refract the view vector through the surface normal; We use -V_world because 'refract' expects the direction FROM the light source
            vec3 ray_primary = refract(-V_world, N_world, eta);

            // Account for Thickness (The "Shift"); For thick volumes, the exit point is shifted based on thickness (common glTF approximation)
            vec3 refraction_dir;
            if (final_thickness > EPSILON) {
                // Approximate the shift as the ray travels through the volume
                refraction_dir = normalize(ray_primary); 
            }
            else {
                // Thin-walled (like a bubble) doesn't shift, it just bends
                refraction_dir = ray_primary;
            }

            // Sample the background with the shifted vector; Using lod based on roughness for 'frosted' glass look
            vec3 background_light = textureLod(prefiltered_map, refraction_dir, lod).rgb;

            // Final coloring with Transmittance (Beer's Law)
            vec3 refracted_light = background_light * final_transmittance;

            // Fresnel Weighting (1 - F), using the BRDF results to ensure we don't transmit light where we should be reflecting
            vec3 F_surface = final_F0 * brdf.x + brdf.y;
            transmission_ibl = refracted_light * (vec3(1.0) - F_surface) * final_transmission;
        }

        // --- COMBINE BASE LAYER ---
        vec3 ibl_base = diffuse_ibl + specular_ibl + transmission_ibl;
        
        // --- SHEEN IBL ---
        vec3 ibl_sheen = vec3(0.0);
        if (dot(final_sheen_color, final_sheen_color) > EPSILON) {
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
    
        // --- CLEARCOAT IBL ---
        vec3 ibl_clearcoat = vec3(0.0);
        if (final_clearcoat > EPSILON) {
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
            ibl_clearcoat = cc_prefiltered * (F0_cc * cc_brdf.x + cc_brdf.y) * final_clearcoat;
        
            // Attenuate base layer by clearcoat's Fresnel
            vec3 F_cc_ibl = schlick_fresnel(NdotV_cc, F0_cc);
            ibl_base = ibl_base * (vec3(1.0) - F_cc_ibl * final_clearcoat) + ibl_clearcoat;
        }
    
        // FINAL IBL
        vec3 ibl_contribution = ibl_base * final_occlusion * ibl_intensity;
        total_lighting += ibl_contribution; // Alternative: direct addition without highpass filter
    }
    
    // ====================================================================================================
    // --- DISCARD ACCORDING TO BLEND MODE ---
    if (material.alpha_mode == 0) {                                  // OPAQUE_MODE
        if (final_alpha < (1.0 - EPSILON) && final_transmission <= EPSILON) {
            discard; // Only discard if it's truly supposed to be transparent
        }
    }
    else if (material.alpha_mode == 1) {                             // MASK_MODE
        if (final_alpha < material.alpha_cutoff) {
            discard;
        }
    }
    else if (material.alpha_mode == 2) {                             // BLEND_MODE (alpha)
        // No discard; already handled in the alpha calculation
    }

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