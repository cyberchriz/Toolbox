#version 450

#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform push_constants {
    mat4 projection;
    mat4 view;
    float roughness;
};

layout(location = 2) in vec3 inPosition;

layout(location = 0) out vec3 outWorldPos; // World space direction vector

void main() {
    // 1. Calculate the position in View Space.
    // This applies the face-specific rotation (view matrix).
    vec4 viewPos = view * vec4(inPosition, 1.0);
    
    // 2. Project to Clip Space for rendering.
    gl_Position = projection * viewPos;
    
    // For skybox rendering, setting Z=W ensures maximum depth (1.0).
    gl_Position.z = gl_Position.w;
    
    // 3. Pass the *View Space* position to the Fragment Shader.
    // This is the correctly rotated direction vector for the current face.
    outWorldPos = viewPos.xyz;
}