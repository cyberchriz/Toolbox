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
    vec4    ambient;
    vec4    diffuse;
    vec4    specular;
    vec4    transmittance;
    vec4    emission;
    float   shininess;
    float   ior;
    float   dissolve;
    int     illum;
    float   roughness;
};

// Define storage buffer (using std430)
layout(binding = 0, set = 0, std430) readonly buffer material_buffer { Material materials[];};
layout(binding = 1, set = 0) uniform sampler2D diffuse_sampler;

// The push constant block for camera and light data.
layout(push_constant) uniform PushConstants {
    mat4    model;
    mat4    view;
    mat4    projection;
    vec4    camera_position;
    vec4    light_position;
    vec4    light_color;
    uint    material_index;
} push_constants;

// Input variables from the vertex shader.
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec3 v_color;
layout(location = 3) in vec3 v_view_dir;    // View direction (World Pos -> Camera Pos)
layout(location = 4) in vec2 v_tex_coord;

// The output color.
layout(location = 0) out vec4 out_color;

void main() {
    const float EPSILON = 0.0001;
    
    // --- MATERIAL SELECTION ---
    Material material = materials[push_constants.material_index];
    
    // --- TEXTURE SAMPLING ---
    // Sample the diffuse texture using the interpolated UV coordinates.
    vec3 texture_color = vec3(texture(diffuse_sampler, v_tex_coord));

    // --- LIGHT COLOR AVAILABILITY CHECK ---
    vec3 final_light_color = vec3(1.0, 1.0, 1.0); // use this default (white light) if the color is not set (zeroed)
    if (!all(lessThan(abs(vec3(push_constants.light_color)), vec3(EPSILON)))) {
        // use the explicit color provided by the push constant
        final_light_color = vec3(push_constants.light_color);
    }

    // --- AMBIENT LIGHTING ---
    vec3 ambient_light_base = vec3(0.2, 0.2, 0.2) * final_light_color; // use this default if the ambient light isn't set
    
    if (!all(lessThan(abs(vec3(ambient_light_base)), vec3(EPSILON)))) {
        // use the explicit ambient light provided by the material
        ambient_light_base = vec3(material.ambient) * final_light_color;
    }
    vec3 ambient_light = ambient_light_base * texture_color;

    // --- DIFFUSE LIGHTING ---
    vec3 light_dir = normalize(vec3(push_constants.light_position) - v_position);
    vec3 norm = normalize(v_normal);
    float diff = max(dot(norm, light_dir), 0.0);

    // Diffuse light is tinted by the texture color and the light color
    vec3 diffuse_light = diff * texture_color * vec3(material.diffuse) * final_light_color;

    // --- SPECULAR LIGHTING ---
    // Represents the shiny highlight on the surface.
    // Use the interpolated v_view_dir (vertex to camera)
    vec3 view_dir = normalize(v_view_dir);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec_strength = pow(max(dot(v_view_dir, reflect_dir), 0.0), material.shininess);

    // Specular light is tinted by the material specular color and the light color
    vec3 specular_light = spec_strength * vec3(material.specular) * final_light_color;

    // --- EMISSION ---
    vec3 emission_light = vec3(material.emission);

    // --- FINAL COLOR CALCULATION ---
    // The final color is a combination of all lighting components and the material's properties.
    vec3 final_color_vec = (ambient_light + diffuse_light + specular_light + emission_light);
    out_color = vec4(final_color_vec, material.dissolve);
}