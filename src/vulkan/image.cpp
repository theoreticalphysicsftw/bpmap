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


#include "image.hpp"

namespace bpmap
{
    VkFormat to_vk_format(vk_image_format_t format)
    {
        static const VkFormat format_table[] =
        {
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_FORMAT_R32G32B32A32_SFLOAT
        };

        return format_table[(uint32_t)(format)];
    }

    VkImageTiling to_vk_tiling(vk_image_tiling_t tiling)
    {
        static const VkImageTiling tiling_table[] =
        {
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_TILING_LINEAR
        };

        return tiling_table[(uint32_t)(tiling)];
    }

    error_t vk_image_t::create(
                                VkDevice device,
                                VmaAllocator allocator,
                                uint32_t width,
                                uint32_t height,
                                vk_image_format_t format,
                                vk_image_tiling_t tiling,
                                bool on_gpu,
                                uint32_t usage,
                                uint32_t samples,
                                uint32_t mips,
                                uint32_t layers
                              )
    {
        VkImageCreateInfo ici = {};
        ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        ici.pNext = nullptr;
        ici.flags = 0;
        ici.imageType = VK_IMAGE_TYPE_2D;
        ici.arrayLayers = layers;
        ici.mipLevels = mips;
        ici.samples = VkSampleCountFlagBits(samples);
        ici.format = to_vk_format(format);
        ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        ici.tiling = to_vk_tiling(tiling);
        ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        ici.usage = usage;
        ici.pQueueFamilyIndices = nullptr;
        ici.queueFamilyIndexCount = 0;
        ici.extent = { width, height, 1 };

        VmaAllocationCreateInfo aci = {};
        aci.pool = VK_NULL_HANDLE;
        aci.flags = 0;
        aci.preferredFlags = 0;
        aci.requiredFlags = 0;
        aci.pUserData = nullptr;
        aci.usage = on_gpu? VMA_MEMORY_USAGE_GPU_ONLY : VMA_MEMORY_USAGE_CPU_TO_GPU;
        
        if(
            vmaCreateImage(
                            allocator,
                            &ici,
                            &aci,
                            &image,
                            &allocation,
                            nullptr
                           )
           != VK_SUCCESS
          )
        {
            return error_t::image_creation_fail;
        }

        this->allocator = allocator;

        VkImageSubresourceRange isr = {};
        isr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        isr.baseArrayLayer = 0;
        isr.baseMipLevel = 0;
        isr.layerCount = layers;
        isr.levelCount = mips;

        VkImageViewCreateInfo ivci = {};
        ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ivci.pNext = nullptr;
        ivci.flags = 0;
        ivci.format = to_vk_format(format);
        ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivci.subresourceRange = isr;
        ivci.image = image;
        ivci.components =
        {
            VK_COMPONENT_SWIZZLE_R,
            VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B,
            VK_COMPONENT_SWIZZLE_A
        };

        if(vkCreateImageView(device, &ivci, nullptr, &view) != VK_SUCCESS)
        {
            return error_t::image_view_creation_fail;
        }
                          
        return error_t::success;
    }
}
