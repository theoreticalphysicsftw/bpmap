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
        direction3d_t up;
        direction3d_t left;
        direction3d_t front;
        point3d_t origin;

        float_t aspect_ratio;
        float_t near;
        float_t far;
    };

    struct scene_settings_t
    {
        camera_t camera;
        float_t resolution_x = 1920;
        float_t resolution_y = 1080;
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
