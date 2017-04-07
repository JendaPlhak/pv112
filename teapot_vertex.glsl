#version 330

in vec4 position;
in vec3 normal;
in vec2 tex_coord;

uniform mat4 model_matrix;
uniform mat4 PVM_matrix;
uniform mat3 normal_matrix;

out vec3 VS_normal_ws;
out vec3 VS_position_ws;
out vec2 VS_tex_coord;

void main()
{
    VS_tex_coord = tex_coord;

    VS_position_ws = vec3(model_matrix * position);
    VS_normal_ws = normalize(normal_matrix * normal);
    gl_Position = PVM_matrix * position;
}
