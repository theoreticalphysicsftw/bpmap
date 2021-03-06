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


#ifndef SCENE_HPP
#define SCENE_HPP

#include <algebra.hpp>
#include <common.hpp>

#include "geometry.hpp"
#include "lights.hpp"
#include "material.hpp"

namespace bpmap
{
    struct camera_t
    {
        // All vec3s should be aligned at 16 bytes GPU side.
        direction3d_t up;
        float_t field_of_view;
        direction3d_t left;
        float_t aspect_ratio;
        direction3d_t front;
        float_t near;
        point3d_t origin;
        float_t far;
    };

    struct scene_settings_t
    {
        camera_t camera;

        uint32_t samples_per_pixel;
        uint32_t light_samples;
        uint32_t max_reflection_bounces;
        uint32_t resolution_x;
        uint32_t resolution_y;
        uint32_t pad[3];
    };

    struct scene_t
    {
        // Holds all the visible objects in the scene each of which references
        // the segment in the triangles array that holds it's indices and index
        // to its material.
        darray_t<visible_object_t> objects;

        darray_t<triangle_t> triangles;

        darray_t<point3d_t> vertices;
        darray_t<codirection3d_t> normals;
        darray_t<point2d_t> texcoords;

        darray_t<material_t> materials;

        darray_t<light_t> lights;

        scene_settings_t settings;
    };
}


#endif // SCENE_HPP
