#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_scalar_block_layout : require

// Input vertex attributes from the Mesh class.
layout(location = 0) in vec4 in_color;
layout(location = 1) in vec4 in_tangent;
layout(location = 2) in vec3 in_position;
layout(location = 3) in vec3 in_normal;
layout(location = 4) in vec2 in_tex_coord;
layout(location = 5) in uint in_material_index;

// Output to the fragment shader.
layout(location = 0) out vec4 v_color;
layout(location = 1) out vec4 v_tangent;
layout(location = 2) out vec3 v_position;
layout(location = 3) out vec3 v_normal;
layout(location = 4) out vec3 v_view_dir;
layout(location = 5) out vec2 v_tex_coord;
layout(location = 6) flat out uint v_material_index;


// Define storage buffer for model matrices (Binding 3, as defined in the descriptor set)
layout(binding = 3, set = 0, std430) readonly buffer model_matrices_buffer { 
    mat4 model_matrices[];
};

// Push constants for dynamic data (using std430)
layout(push_constant) uniform push_constants {
	mat4 	view; // Camera's view transform
	mat4 	projection;
	uint 	material_index;
	uint 	lights_count;
	uint    prefiltered_mip_levels;
	float   exposure;
	float   ibl_intensity;
	vec3    ambient_scene_color;
	vec3 	camera_position; // World Space Camera Position
};

void main() {
    // Retrieve the Model Matrix specific to this instance using gl_InstanceIndex
    mat4 model = model_matrices[gl_InstanceIndex];

    // Transform the vertex position to Clip Space and pass the vertex's world space position to the fragment shader
    vec4 world_pos = model * vec4(in_position, 1.0);
    gl_Position = projection * view * world_pos;
    v_position = vec3(world_pos);

    // --- VIEW DIRECTION CALCULATION ---
    v_view_dir = camera_position.xyz - vec3(world_pos);

    // --- TBN BASIS TRANSFORMATION ---
    // Normal Matrix: Inverse Transpose of the Model Matrix's 3x3 component.
    mat3 normal_matrix = transpose(inverse(mat3(model)));

    // 1. Transform the Normal vector
    // This value will be interpolated and then normalized in the fragment shader.
    v_normal = normal_matrix * in_normal;
    
    // 2. Transform the Tangent vector's direction (xyz) 
    v_tangent.xyz = normal_matrix * in_tangent.xyz;
    v_tangent.w = in_tangent.w;

    // --- OTHER INTERFACE PASS-THROUGH TO NEXT STAGE (FRAGMENT SHADER) ---
    v_color = in_color;
    v_material_index = in_material_index;
    v_tex_coord = in_tex_coord;
}
