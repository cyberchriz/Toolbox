#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_shader_8bit_storage : require
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : require
#extension GL_EXT_shader_16bit_storage : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : require
#extension GL_EXT_scalar_block_layout : require

// We define the same struct as the C++ side: This struct will be an array within the storage buffer.
struct Material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 transmittance;
    vec4 emission;
    float shininess;
    float ior;
    float dissolve;
    int   illum;
    float roughness;
};

// Define storage buffer (using std430)
layout(binding = 0, set = 0, std430) readonly buffer MaterialBuffer { Material materials[];} materialBuffer;

// The push constant block for camera and light data.
layout(push_constant) uniform PushConstants {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 light_position;
    vec4 view_position;
    uint material_index;
} push_constants;

// Input variables from the vertex shader.
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec3 v_color;
layout(location = 3) in vec3 v_view_dir;
layout(location = 4) in vec2 v_tex_coord;

// The output color.
layout(location = 0) out vec4 out_color;

void main() {
    // --- MATERIAL SELECTION ---
    Material material = materialBuffer.materials[push_constants.material_index];
    
    // --- AMBIENT LIGHTING ---
    vec3 ambient_light = vec3(material.ambient);

    // --- DIFFUSE LIGHTING ---
    vec3 light_dir = normalize(vec3(push_constants.light_position) - v_position);
    vec3 norm = normalize(v_normal);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse_light = diff * vec3(material.diffuse);

    // --- SPECULAR LIGHTING ---
    // Represents the shiny highlight on the surface.
    vec3 view_dir = normalize(vec3(push_constants.view_position) - v_position);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec_strength = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular_light = spec_strength * vec3(material.specular);

    // --- EMISSION ---
    vec3 emission_light = vec3(material.emission);

    // --- FINAL COLOR CALCULATION ---
    // The final color is a combination of all lighting components and the material's properties.
    vec3 final_color_vec = (ambient_light + diffuse_light + specular_light + emission_light);
    out_color = vec4(final_color_vec, material.dissolve);

}