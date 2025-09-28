#version 450
#extension GL_ARB_separate_shader_objects : enable

// Input vertex attributes from the Mesh class (must match the order: position, normal, tex_coord, color).
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_tex_coord;
layout(location = 3) in vec3 in_color;

// Output to the fragment shader. The locations must match.
layout(location = 0) out vec3 f_position;
layout(location = 1) out vec3 f_normal;
layout(location = 2) out vec3 f_color;
layout(location = 3) out vec3 f_view_dir;
layout(location = 4) out vec2 f_tex_coord;

// Push constants for dynamic data like camera and model matrices.
layout(push_constant) uniform push_constants {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 camera_position;
    vec4 light_position;
    vec4 light_color;
    uint material_index;
};

void main() {
    // Transform the vertex position by the model, view, and projection matrices
    vec4 world_pos = model * vec4(in_position, 1.0);
    gl_Position = projection * view * world_pos;

    // Pass the vertex's position in world space to the fragment shader
    f_position = vec3(world_pos);

    // Transform the normal by the model matrix to get it into world space (using a mat3 to avoid any translation components)
    f_normal = mat3(model) * in_normal;

    // Pass the vertex color directly.
    f_color = in_color;

    // Calculate the view direction for specular lighting.
    f_view_dir = vec3(camera_position) - f_position;

    // Pass the vertex UV coordinates to the fragment shader.
    // VULKAN FIX: Flip the V-coordinate (Y-axis) to align with image loading conventions (top-left origin).
    f_tex_coord = vec2(in_tex_coord.x, 1.0 - in_tex_coord.y);
}