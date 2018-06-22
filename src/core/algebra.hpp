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


#ifndef ALGEBRA_HPP
#define ALGEBRA_HPP

#include "common.hpp"

namespace bpmap
{
    template<typename T, size_t size, bool covariant>
    struct vector_t
    {
        using value_t = T;

        value_t components[size];

        // TODO: Check if this is unrolled by the compiler and if it is not
        // then do it manually.
        #define BPMAP_IMPLEMENT_VECTOR_OPERATOR(OP) \
        vector_t operator OP (vector_t& rhs) \
        { \
            vector_t<value_t, size, false> result; \
            \
            for(auto i = 0u; i < size; ++i) \
            { \
                result.components[i] = components[i] OP rhs.components[i]; \
            } \
            \
            return result; \
        }

        BPMAP_IMPLEMENT_VECTOR_OPERATOR(+)
        BPMAP_IMPLEMENT_VECTOR_OPERATOR(-)
        // Hadamard product
        BPMAP_IMPLEMENT_VECTOR_OPERATOR(*)

        #undef BPMAP_IMPLEMENT_VECTOR_OPERATOR


        #define BPMAP_IMPLEMENT_VECTOR_ASSIGN_OPERATOR(OP) \
        vector_t& operator OP (vector_t& rhs) \
        { \
            for(auto i = 0u; i < size; ++i) \
            { \
                components OP rhs.components; \
            } \
        } \

        BPMAP_IMPLEMENT_VECTOR_ASSIGN_OPERATOR(+=)
        BPMAP_IMPLEMENT_VECTOR_ASSIGN_OPERATOR(-=)
        BPMAP_IMPLEMENT_VECTOR_ASSIGN_OPERATOR(*=)

        #undef BPMAP_IMPLEMENT_VECTOR_ASSIGN_OPERATOR

        T& operator[](size_t i)
        {
            return components[i];
        }

        T& operator[](size_t i) const
        {
            return components[i];
        }

    };

    template<typename T, size_t size_0, size_t size_1>
    struct transform_t
    {
        using value_t = T;

        value_t components[size_0 * size_1];

        #define BPMAP_IMPLEMENT_TRANSFORM_OPERATOR(OP) \
        transform_t operator OP (transform_t& rhs) \
        { \
            transform_t result; \
            \
            auto size = size_0 * size_1;\
            \
            for(auto i = 0u; i < size; ++i) \
            { \
                result.components[i] = components[i] OP rhs.components[i]; \
            } \
            \
            return result; \
        }

        BPMAP_IMPLEMENT_TRANSFORM_OPERATOR(+)
        BPMAP_IMPLEMENT_TRANSFORM_OPERATOR(-)

        #undef BPMAP_IMPLEMENT_TRANSFORM_OPERATOR
    };

    template<typename T, size_t size>
    struct point_t
    {
        using value_t = T;

        value_t components[size];

        // Only point subtraction a.k.a finding the vector of the direction of
        // the affine space that transforms the first point into the other point
        // makes sense.
        vector_t<value_t, size, false> operator-(point_t& rhs)
        {
            vector_t<value_t, size, false> result;

            // TODO: Check if this is unrolled by the compiler and if it is not
            // then do it manually.
            for(auto i = 0u; i < size; ++i)
            {
                result.components[i] = components[i] - rhs.components[i];
            }

            return result;
        }

        template<bool covariant>
        operator vector_t<T, size, covariant>()
        {
            vector_t<T, size, covariant> result;

            for(auto i = 0u; i < size; ++i)
            {
                result.components[i] = components[i];
            }

            return result;
        }

        T& operator[](size_t i)
        {
            return components[i];
        }

        T& operator[](size_t i) const
        {
            return components[i];
        }
    };

    using point3d_embedded_t = point_t<float_t, 4>;
    using direction3d_embedded_t = vector_t<float_t, 4, false>;
    using codirection3d_embedded_t = vector_t<float_t, 4, true>;
    using transform3d_embedded_t = transform_t<float_t, 4, 4>;

    using point3d_t = point_t<float_t, 3>;
    using direction3d_t = vector_t<float_t, 3, false>;
    using codirection3d_t = vector_t<float_t, 3, true>;
    using transform3d_t = transform_t<float_t, 3, 3>;


    using point2d_t = point_t<float_t, 2>;
    using direction2d_t = vector_t<float_t, 2, false>;
    using codirection2d_t = vector_t<float_t, 2, true>;
    using transform_2d_t = transform_t<float_t, 2, 2>;

    using color4d_t = vector_t<uint8_t, 4, false>;
    using color3df_t = vector_t<float_t, 3, false>;
}

#endif // ALGEBRA_HPP
