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


#include "vulkan.hpp"
#include "sampler.hpp"

#include <functional>


namespace bpmap::vk
{
    size_t sampler_desc_hash_t::operator()(const sampler_desc_t& desc) const
    {
        string_view_t sv((const char*)&desc, sizeof(desc));

        return std::hash<string_view_t>()(sv);
    }

    error_t sampler_t::create(const device_t& device, const sampler_desc_t& desc)
    {
        static VkBorderColor color_table[] =
        {
            VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
            VK_BORDER_COLOR_INT_TRANSPARENT_BLACK,
            VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
            VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
            VK_BORDER_COLOR_INT_OPAQUE_WHITE,            
        };

        VkSamplerCreateInfo sci = {};
        sci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sci.pNext = nullptr;
        sci.flags = 0;
        sci.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sci.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sci.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sci.anisotropyEnable = (desc.anisotropy > 0)? VK_TRUE : VK_FALSE;
        sci.maxAnisotropy = desc.anisotropy;
        sci.mipmapMode =
            desc.is_linear? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
        sci.mipLodBias = desc.lod_bias;
        sci.compareEnable = VK_FALSE;
        sci.minFilter = desc.is_linear? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
        sci.magFilter = desc.is_linear? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
        sci.minLod = desc.min_lod;
        sci.maxLod = desc.max_lod;
        sci.borderColor = color_table[(size_t)desc.border_color];

        if(vkCreateSampler(device.get_device(), &sci, nullptr, &sampler) != VK_SUCCESS)
        {
            return error_t::sampler_creation_fail;
        }

        dev = &device;

        slot = device.bind(this);
        return error_t::success;
    }

    sampler_t::sampler_t() :
        dev(nullptr),
        sampler(VK_NULL_HANDLE),
        slot(INVALID_SLOT)
    {
    }

    sampler_t::~sampler_t()
    {
        if (dev)
        {
            vkDestroySampler(dev->get_device(), sampler, nullptr);
        }
    }
}
