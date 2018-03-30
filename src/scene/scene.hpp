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

#include "common.hpp"
#include "geometry.hpp"
#include "lights.hpp"
#include "material.hpp"

namespace bpmap
{
    struct scene_t
    {
        // Holds all agregates in the scene.
        darray_t<aggregate_object_t> aggregates;

        // Holds all the visible objects in the scene each of which references
        // the segment in the triangles array that holds it's indices and index
        // to its material.
        darray_t<visible_object_t> objects;

        // Holds indices to the vertex_attributes array.
        darray_t<uint32_t> triangles;

        darray_t<vertex_attribute_t> vertex_attributes;

        darray_t<material_t> materials;

        darray_t<light_t> lights;
    };
}


#endif // SCENE_HPP
