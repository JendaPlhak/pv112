#version 130

out vec4 final_color;

in vec3 VS_normal_ws;
in vec3 VS_position_ws;
in vec2 VS_tex_coord;

uniform vec3 material_ambient_color;
uniform vec3 material_diffuse_color;
uniform vec3 material_specular_color;
uniform float material_shininess;

uniform vec4 light1_position;
uniform vec4 light2_position;
uniform vec3 light_ambient_color;
uniform vec3 light_diffuse_color;
uniform vec3 light_specular_color;

uniform vec3 eye_position;

uniform sampler2D my_tex;
uniform sampler2D rocks_tex;
uniform sampler2D wood_tex;
uniform sampler2D dice_tex;

void main()
{
    vec3 dice_color = texture(dice_tex, VS_tex_coord).rgb;
	vec3 wood_color = texture(wood_tex, VS_tex_coord).rgb;
	vec3 rock_color = texture(rocks_tex, VS_tex_coord).rgb;
    vec3 tex_color = wood_color;

    vec3 N = normalize(VS_normal_ws);
    vec3 Eye = normalize(eye_position - VS_position_ws);

    vec3 L1 = normalize(light1_position.xyz - VS_position_ws * light1_position.w);
    vec3 L2 = normalize(light2_position.xyz - VS_position_ws * light2_position.w);
    vec3 L = normalize(L1 + L2);

    vec3 H = normalize(L + Eye);

    float Idiff = max(dot(N, L), 0.0);
    float Ispec = pow(max(dot(N, H), 0.0), material_shininess) * Idiff;

    vec3 mat_ambient = material_ambient_color;
    vec3 mat_diffuse = material_diffuse_color;
    vec3 mat_specular = material_specular_color;

    vec3 light =
        mat_ambient * light_ambient_color * tex_color +
        mat_diffuse * light_diffuse_color * Idiff * tex_color +
        mat_specular * light_specular_color * Ispec;

    final_color = vec4(light, 1.0);
}
