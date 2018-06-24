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


#ifndef LIGHTS_HPP
#define LIGHTS_HPP

#include "common.hpp"
#include "algebra.hpp"

namespace bpmap
{
    struct rect_light_t
    {
        point3d_t point;
        codirection3d_t normal;
        direction3d_t basis_vec0;
        direction3d_t basis_vec1;
        color3df_t color;
        float_t param0_max;
        float_t param1_max;

        float_t power;
    };

    using light_t = rect_light_t;
}

#endif // LIGHTS_HPP
