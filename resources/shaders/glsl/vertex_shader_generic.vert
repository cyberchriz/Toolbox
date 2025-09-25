#version 450
#extension GL_ARB_separate_shader_objects : enable

// Input vertex attributes from the Mesh class (must match the order: position, normal, tex_coord, color).
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inColor;

// Output to the fragment shader. The locations must match.
layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragColor;
layout(location = 3) out vec3 fragViewDir;
layout(location = 4) out vec2 fragTexCoord;

// Push constants for dynamic data like camera and model matrices.
layout(push_constant) uniform PushConstants {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 light_position;
    vec4 view_position;
    uint material_index;
} pushConstants;

void main() {
    // Transform the vertex position by the model, view, and projection matrices.
    
    vec4 worldPos = pushConstants.model * vec4(inPosition, 1.0);
    gl_Position = pushConstants.projection * pushConstants.view * worldPos;

    // Pass the vertex's position in world space to the fragment shader.
    
    fragPosition = vec3(worldPos);

    // Transform the normal by the model matrix to get it into world space.
    // We use a mat3 to avoid any translation components.
    
    fragNormal = mat3(pushConstants.model) * inNormal;

    // Pass the vertex color directly.
    fragColor = inColor;

    // Calculate the view direction for specular lighting.
    // This is the vector from the vertex to the camera's position.
    // Since we don't have the camera position, we can calculate it from the view matrix.
    // The inverse of the view matrix gives us the camera's world space coordinates in the last column.

    mat4 inverseView = inverse(pushConstants.view);
    vec3 cameraPos = vec3(inverseView[3]);
    fragViewDir = cameraPos - fragPosition;
    fragTexCoord = inTexCoord; // directly pass the texture coordinates to the fragment shader
}