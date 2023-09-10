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


#ifndef COMMON_HPP
#define COMMON_HPP

#if defined(BPMAP_DEBUG)
    #include<iostream>
#endif

#include <cstdint>
#include <cstring>

#include <string>
#include <string_view>

#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <array>

#include <variant>
#include <utility>

#if defined(_MSC_VER)
    #define BMAP_FORCE_INLINE __forceinline
#elif defined(__GNUG__) || defined(__clang__)
    #define BMAP_FORCE_INLINE __attribute__((always_inline))
#else
    #define BMAP_FORCE_INLINE
#endif

namespace bpmap
{
    using bool_t = bool;
    using int_t =  int;
    using char_t = char;
    using float_t = float;
    using double_t = double;
    using string_t = std::string;
    using string_view_t = std::string_view;

    template <typename T>
    using darray_t = std::vector<T>;

    template <typename T, size_t N>
    using array_t = std::array<T,N>;

    template <typename T>
    using deque_t = std::deque<T>;

    template <class... U>
    using variant_t = std::variant<U...>;

    template <typename T, typename U>
    using pair_t = std::pair<T,U>;

    template<typename K, typename V, typename H = std::hash<K>>
    using hash_table_t = std::unordered_map<K,V,H>;

    template<typename T>
    using hash_set_t = std::unordered_set<T>;

}

#endif // COMMON_HPP
