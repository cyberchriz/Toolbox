#version 450

#extension GL_ARB_separate_shader_objects : enable

// The source texture (converted 6-face cubemap from Step 1)
layout(binding = 0) uniform samplerCube environmentMap;

layout(location = 0) in vec3 inWorldPos;

layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;
const uint NUM_SAMPLES = 1024u; // Number of Monte Carlo samples

// Function to generate a random vector uniformly distributed over a hemisphere
// We'll use a pre-calculated sampling vector based on the sample index.
vec3 SampleHemisphere(float xi1, float xi2, vec3 normal) {
    // Uniformly sample a point on the hemisphere
    float phi = 2.0 * PI * xi1;
    float cosTheta = xi2;
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
    
    // Construct TBN matrix for hemisphere alignment
    vec3 up = abs(normal.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    
    // Transform H to world space
    return TBN * H;
}

// Simple deterministic pseudo-random sequence based on fragment coordinate and sample index
float RandomSequence(uint sample_id, uint index) {
    uint seed = uint(gl_FragCoord.x * 1920.0 + gl_FragCoord.y) + sample_id + index * 101;
    seed = (seed * 1103515245u + 12345u) & 0x7FFFFFFFu;
    return float(seed) / float(0x7FFFFFFF);
}

void main() {
    vec3 N = normalize(inWorldPos);
    vec3 irradiance = vec3(0.0);
    
    float sample_delta = 1.0 / float(NUM_SAMPLES);
    float acc_weight = 0.0;

    for (uint i = 0u; i < NUM_SAMPLES; ++i) {
        // Use pseudo-random values for Monte Carlo
        float xi1 = RandomSequence(i, 0u);
        float xi2 = RandomSequence(i, 1u);
        
        vec3 H = SampleHemisphere(xi1, xi2, N);
        
        // Direction is the sampled vector H
        vec3 sampleDir = H;
        
        // Calculate the cosine weight (required for Monte Carlo integration)
        float NdotH = max(dot(N, sampleDir), 0.0);
        
        if (NdotH > 0.0) {
            // LOD 0 (highest detail) is used for irradiance, as it integrates over a large area
            irradiance += textureLod(environmentMap, sampleDir, 0.0).rgb * NdotH;
            acc_weight += NdotH;
        }
    }
    
    // Final irradiance = integral / sum of weights (for normalization)
    outColor = vec4(irradiance / acc_weight, 1.0);
}
