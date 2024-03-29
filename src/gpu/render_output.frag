// Copyright 2018 Mihail Mladenov
//
// This file is part of bpmap.
//
// bpmap is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// bpmap is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with bpmap.  If not, see <http://www.gnu.org/licenses/>.


#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../vulkan/vk.glslh"

layout(push_constant) uniform push_range
{
    uint rendered_image_id;
    uint sampler_id;
};

layout(location = 0) in vec4 fragment_data;
layout(location = 1) in vec2 fragment_uv;

layout(location = 0) out vec4 output_color;

void main()
{
    vec4 image_color = VK_SAMPLE_2D(rendered_image_id, sampler_id, fragment_uv);
    float a = fragment_data.x;
    float gamma = fragment_data.y;
    output_color = vec4(a * pow(image_color.xyz, vec3(gamma, gamma, gamma)), 1.0);
}
