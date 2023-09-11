// Copyright 2023 Mihail Mladenov
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


#ifndef SHADER_REGISTRY_HPP
#define SHADER_REGISTRY_HPP

#include "core/io.hpp"
#include "vulkan.hpp"

namespace bpmap::vk
{
    class shader_registry_t
    {
        // We don't want registering new shaders to invalidate
        // others by calling destructors.
        deque_t<shader_t> shaders; 
        hash_table_t<string_t, shader_id_t> name_to_id;

        const device_t* dev;

        public:
        shader_registry_t(const device_t& device) : dev(&device) {}

        error_t add(
                     const string_t& name,
                     const uint32_t* code,
                     size_t size,
                     shader_stage_t type
                   );

        using desc_array_t = darray_t<pair_t<string_t, shader_stage_t>>;
        error_t add_from_file(const string_t& name, shader_stage_t type);
        error_t add_from_file(const desc_array_t& descs);

        shader_t& get(const string_t& name);
    };
}

#endif
