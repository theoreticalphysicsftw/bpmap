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


#ifndef VULKAN_SAMPLER_HPP
#define VULKAN_SAMPLER_HPP


namespace bpmap
{
    enum class vk_sampler_border_t
    {
        transparent_black_float = 0,
        transparent_black_int,
        opaque_black_float,
        opaque_black_int,
        opaque_white_float,
        opaque_white_int,
    };

    struct vk_sampler_desc_t
    {
        bool_t is_linear = true;
        float_t lod_bias = 0;
        float_t min_lod = 0;
        float_t max_lod = 0;
        uint32_t anisotropy = 16;
        vk_sampler_border_t border_color = vk_sampler_border_t::opaque_black_float;

        bool_t operator==(const vk_sampler_desc_t&) const = default;
    };

    struct vk_sampler_desc_hash_t
    {
        size_t operator()(const vk_sampler_desc_t& desc) const;
    };

    class vk_sampler_t
    {
        const vk_device_t* dev;
        VkSampler sampler;
        
        vk_sampler_t(const vk_sampler_t& other) = delete;
        vk_sampler_t& operator=(const vk_sampler_t& other) = delete;

        public:
        VkSampler get_handle() const { return sampler; }
        
        error_t create(const vk_device_t& device, const vk_sampler_desc_t& desc);

        vk_sampler_t();
        ~vk_sampler_t();
    };
}

#endif
