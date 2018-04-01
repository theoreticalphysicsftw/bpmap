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


#include "gui_renderer.hpp"

namespace bpmap
{
    error_t gui_renderer_t::setup_font_texture()
    {

        VkImageSubresourceRange isr = {};
        isr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        isr.baseArrayLayer = 0;
        isr.baseMipLevel = 0;
        isr.layerCount = 1;
        isr.levelCount = 1;

        VkImageViewCreateInfo ivci = {};
        ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ivci.pNext = nullptr;
        ivci.flags = 0;
        ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
        ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivci.subresourceRange = isr;

        if(vulkan->create_image_view(font_view,ivci) != error_t::success)
        {
            return error_t::font_texture_setup_fail;
        }

        VkSamplerCreateInfo sci = {};
        sci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sci.pNext = nullptr;
        sci.flags = 0;
        sci.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sci.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sci.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sci.anisotropyEnable = VK_FALSE;
        sci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sci.mipLodBias = 0.0;
        sci.compareEnable = VK_FALSE;
        sci.minFilter = VK_FILTER_LINEAR;
        sci.magFilter = VK_FILTER_LINEAR;
        sci.minLod = 0.0;
        sci.maxLod = 0.0;
        sci.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

        return vulkan->create_sampler(font_sampler, sci);
    }


    void gui_renderer_t::bind_gui(const gui_t &g)
    {
        gui = &g;
    }

    void gui_renderer_t::bind_vulkan(const vulkan_t &vk)
    {
        vulkan = &vk;
    }

    error_t gui_renderer_t::init()
    {
        setup_font_texture();

        return error_t::success;
    }

    gui_renderer_t::~gui_renderer_t()
    {
        vulkan->destroy_image_view(font_view);
        vulkan->destroy_sampler(font_sampler);
    }
}
