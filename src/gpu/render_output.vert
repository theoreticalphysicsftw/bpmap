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

layout(location = 0) out vec4 fragment_color;
layout(location = 1) out vec2 fragment_uv;

vec2 positions[6] = vec2[](
                            vec2(-1.0, -1.0),
                            vec2(1.0, -1.0),
                            vec2(1.0, 1.0),
                            vec2(-1.0, -1.0),
                            vec2(1.0, 1.0),
                            vec2(-1.0, 1.0)
                          );

vec2 uvs[6] = vec2[](
                      vec2(0.0, 0.0),
                      vec2(1.0, 0.0),
                      vec2(1.0, 1.0),
                      vec2(0.0, 0.0),
                      vec2(1.0, 1.0),
                      vec2(0.0, 1.0)
                    );

void main()
{
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragment_color = vec4(1.0, 1.0, 1.0, 1.0);
    fragment_uv = uvs[gl_VertexIndex];
}
