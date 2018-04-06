#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D font_texture;

layout(location = 0) in vec4 fragent_color;
layout(location = 1) in vec2 fragment_uv;

layout(location = 0) out vec4 output_color;

void main()
{
    output_color = fragent_color * texture(font_texture, fragment_uv);
}
