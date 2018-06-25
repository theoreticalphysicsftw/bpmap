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


#include<core/algebra.hpp>
#include<common.hpp>

#ifndef MATERIAL_HPP
#define MATERIAL_HPP

namespace bpmap
{
    struct material_t
    {
        color3df_t base_color;
        float_t roughness;
        float_t metallic;
        uint32_t pad[3];
    };
}

#endif // MATERIAL_HPP
