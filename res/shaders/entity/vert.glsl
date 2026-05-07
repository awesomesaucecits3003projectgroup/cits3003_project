#version 410 core
#include "../common/lights.glsl"

// Per vertex data
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texture_coordinate;

out VertexOut {
    //LightingResult lighting_result; // removed
    vec3 ws_position; // added - gets position of vertex wrt world space
    vec3 ws_normal; // added - gets normal vector of vertex wrt world space
    vec2 texture_coordinate;
} vertex_out;

// Per instance data
uniform mat4 model_matrix;
uniform mat3 normal_matrix;

// Material properties
//uniform vec3 diffuse_tint;
//uniform vec3 specular_tint;
//uniform vec3 ambient_tint;
//uniform float shininess;
uniform float texture_scale;

// Light Data
#if NUM_PL > 0
layout (std140) uniform PointLightArray {
    PointLightData point_lights[NUM_PL];
};
#endif

// Global data
//uniform vec3 ws_view_position;
uniform mat4 projection_view_matrix;

void main() {
    // Transform vertices
    vec3 ws_position = (model_matrix * vec4(vertex_position, 1.0f)).xyz;
    vec3 ws_normal = normalize(normal_matrix * normal);
    vertex_out.texture_coordinate = texture_scale * texture_coordinate; // changed, multiplies vec3 texture_coordinate by float texture_scale to uniformly scale all points

    vertex_out.ws_position = ws_position; // added - sends out vertex pos
    vertex_out.ws_normal = ws_normal; // added - sends out vertex norm vec

    gl_Position = projection_view_matrix * vec4(ws_position, 1.0f);
    
    // Per vertex lighting (moved to fragment shader)
    /* vec3 ws_view_dir = normalize(ws_view_position - ws_position);
    LightCalculatioData light_calculation_data = LightCalculatioData(ws_position, ws_view_dir, ws_normal);
    Material material = Material(diffuse_tint, specular_tint, ambient_tint, shininess);

    vertex_out.lighting_result = total_light_calculation(light_calculation_data, material
        #if NUM_PL > 0
        ,point_lights
        #endif
    ); */
}
