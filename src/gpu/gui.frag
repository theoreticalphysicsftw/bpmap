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


layout(location = 0) in vec4 fragment_color;
layout(location = 1) in vec2 fragment_uv;

layout(location = 0) out vec4 output_color;

layout(push_constant) uniform gui_data
{
    mat4 projection;
    float pad0;
    float pad1;
    uint font_texture_id;
    uint sampler_id;
};

void main()
{
    output_color = fragment_color * VK_SAMPLE_2D(font_texture_id, sampler_id, fragment_uv);
}
