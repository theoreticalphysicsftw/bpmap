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


#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <common.hpp>

#include "algebra.hpp"

namespace bpmap
{

    struct attribute_index_t
    {
        uint32_t vertex_index;
        uint32_t normal_index;
        uint32_t texcoord_index;
    };

    struct triangle_t
    {
        attribute_index_t vertices[3];
        uint32_t material_id;
    };

    // A collection of primitives all of the same type that share the same
    // material.
    struct visible_object_t
    {
        uint32_t first_primitive;
        uint32_t primitives_count;
        uint32_t material_idx;
    };

}

#endif // GEOMETRY_HPP
