#version 330

out vec4 final_color;

in vec2 VS_tex_coord;

uniform sampler2D my_color_tex;
uniform sampler2D my_alpha_tex;

void main()
{
    vec3 tex_color = texture(my_color_tex, VS_tex_coord).rgb;
    float tex_alpha = texture(my_alpha_tex, VS_tex_coord).a;

    //final_color = vec4(tex_color, 1.0);
    final_color = vec4(tex_color, tex_alpha);
}
