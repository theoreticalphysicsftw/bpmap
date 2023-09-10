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


#ifndef SAMPLER_REGISTRY_HPP
#define SAMPLER_REGISTRY_HPP

#include "vulkan.hpp"

namespace bpmap
{
    class sampler_registry_t
    {
        // We don't want registering new samplers to invalidate
        // others by calling destructors.
        deque_t<vk_sampler_t> samplers; 
        hash_table_t<vk_sampler_desc_t, uint32_t, vk_sampler_desc_hash_t> desc_to_id;

        const vk_device_t* dev;

        public:
        sampler_registry_t(const vk_device_t& device) : dev(&device) {}

        error_t add(const vk_sampler_desc_t& desc);
        vk_sampler_t& get(const vk_sampler_desc_t& desc);
    };
}

#endif
