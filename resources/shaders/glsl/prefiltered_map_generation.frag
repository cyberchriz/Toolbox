#version 450

#extension GL_ARB_separate_shader_objects : enable

// Push Constant structure (Roughness is passed as an offset after Proj/View)
layout(push_constant) uniform push_constants {
    mat4 projection;
    mat4 view;
    float roughness; // roughness value for the current mip level
};

// The source texture (converted 6-face cubemap from Step 1)
layout(binding = 0) uniform samplerCube environmentMap;

layout(location = 0) in vec3 inWorldPos;

layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;
const float EPSILON = 0.0001;
const float MIN_ROUGHNESS = 0.01;
const uint NUM_SAMPLES = 1024u; // Monte Carlo samples (can be higher for better quality)

// ---------------------------------------------------------------------------------
// Helper Functions (Same as used in core PBR lighting, but adapted for cubemap)

// Calculates the Normal Distribution Function (NDF) using the GGX/Trowbridge-Reitz model
float DistributionGGX(vec3 N, vec3 H, float a2) {
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return a2 / (denom + EPSILON);
}

// Hammersley sequence used for generating uniform 2D samples
vec2 Hammersley(uint i, uint N) {
    // Radical inverse of 2 (Van der Corput sequence)
    uint bits = i; 
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    float rdi = float(bits) * 2.3283064365386963e-10; // (1.0 / 2^32)
    
    return vec2(float(i) / float(N), rdi);
}

// Importance Sampling based on the GGX Distribution
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
    float a = roughness * roughness;
    
    // Spherical coordinates
    float phi = 2.0 * PI * Xi.x;
    
    // Clamping to prevent sqrt of negative (though rare, it is safer)
    float term = max(1.0 + (a * a - 1.0) * Xi.y, EPSILON); 
    float cosTheta = sqrt((1.0 - Xi.y) / term); 
    
    float sinTheta = sqrt(max(1.0 - cosTheta * cosTheta, 0.0)); // Clamp sinTheta input
    
    // From spherical to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
    
    // Construct TBN matrix to transform H to world space
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
    
    mat3 TBN = mat3(tangent, bitangent, N);
    return TBN * H; // H is the half-vector
}
// ---------------------------------------------------------------------------------

void main() {
    vec3 R = normalize(inWorldPos); // Reflection vector (same as Normal N in this context)
    vec3 V = R;                     // View vector (coincides with R/N, since V is where we "look from")
    vec3 N = R;                     // Normal
    
    // Clamp roughness to prevent NDF/PDF from being zero
    float clamped_roughness = max(roughness, MIN_ROUGHNESS);
    float a2 = clamped_roughness * clamped_roughness;

    vec3 prefiltered_color = vec3(0.0);
    float total_weight = 0.0;
    
    for (uint i = 0u; i < NUM_SAMPLES; ++i) {
        vec2 Xi = Hammersley(i, NUM_SAMPLES);

        vec3 H = ImportanceSampleGGX(Xi, N, clamped_roughness); // Half vector
        vec3 L = normalize(2.0 * dot(V, H) * H - V); // Light/Sample vector (reflection of V around H)
        
        float NdotL = max(dot(N, L), 0.0);
        
        if (NdotL > 0.0) {
            float NdotH = max(dot(N, H), 0.0);
            float VdotH = max(dot(V, H), 0.0);
            
            // PDF (Probability Distribution Function) of the Importance Sampling:
            // D(H) * NdotH / (4 * VdotH)
            float D = DistributionGGX(N, H, a2);

            // Guard against VdotH being zero in the PDF (division by zero)
            float pdf = D * NdotH / max(4.0 * VdotH, EPSILON);
            
            // Weight = NdotL / pdf. We average with NdotL, so weight = NdotL * (1/pdf)
            float weight = NdotL / max(pdf, EPSILON);
            
            // Sampling the environment map using the calculated light vector L
            // The textureLod ensures we sample at LOD 0, as mip levels are controlled by roughness.
            prefiltered_color += textureLod(environmentMap, L, 0.0).rgb * weight;
            total_weight += weight;
        }
    }
    
    outColor = vec4(prefiltered_color / total_weight, 1.0);
}
