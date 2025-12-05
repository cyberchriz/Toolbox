#version 450

#extension GL_ARB_separate_shader_objects : enable

// The source texture (HDR equirectangular map)
layout(binding = 0) uniform sampler2D equirectangularMap;

layout(location = 0) in vec3 inWorldPos;

layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;

void main() {

    // Convert the 3D direction (inWorldPos) to spherical coordinates (UV)
    vec3 N = normalize(inWorldPos);
    
    // Calculate the azimuth angle (phi) and elevation angle (theta)
    float phi = atan(N.z, N.x);
    float theta = acos(N.y);
    
    // Map to UV coordinates (u: azimuth, v: elevation)
    vec2 uv = vec2(phi / (2.0 * PI) + 0.5, theta / PI);
    
    // Sample the 2D texture
    vec3 color = texture(equirectangularMap, uv).rgb;

    outColor = vec4(color, 1.0);
}
