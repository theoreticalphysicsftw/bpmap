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
    // This class contains vertex attributes. The naming matches triangle data,
    // however different primitives can interpret the same memory in different
    // ways.
    struct vertex_attribute_t
    {
        point3d_t vertex;
        codirection3d_t normal;
        // Parametrization coordinates
        float_t u;
        float_t v;
        uint32_t material_id;
    };

    // A collection of primitives all of the same type that share the same
    // material.
    struct visible_object_t
    {
        uint64_t first_primitive;
        uint64_t primitives_count;
        uint64_t material_idx;
    };

}

#endif // GEOMETRY_HPP
