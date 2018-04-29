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

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(binding = 0) uniform gui_data
{
    mat4 projection;
};

layout(location = 0) in vec2 vertex_position;
layout(location = 1) in vec2 vertex_uv;
layout(location = 2) in uvec4 vertex_color;

layout(location = 0) out vec4 fragment_color;
layout(location = 1) out vec2 fragment_uv;


void main()
{
    gl_Position = projection * vec4(vertex_position, 0.0, 1.0);

    fragment_color = vec4(
                           vertex_color[0]/255.0,
                           vertex_color[1]/255.0,
                           vertex_color[2]/255.0,
                           vertex_color[3]/255.0
                          );

    fragment_uv = vertex_uv;
}
