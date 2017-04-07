#version 330

in vec4 position;
in vec2 tex_coord;

uniform mat4 PVM_matrix;

out vec2 VS_tex_coord;

void main()
{
    VS_tex_coord = tex_coord;

    gl_Position = PVM_matrix * position;
}
