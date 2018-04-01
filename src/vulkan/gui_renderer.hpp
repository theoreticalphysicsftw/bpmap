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


#ifndef GUI_RENDERER_HPP
#define GUI_RENDERER_HPP

#include "vulkan.hpp"
#include "gui/gui.hpp"

namespace bpmap
{
    class gui_renderer_t
    {
        vk_image_t font_image;

        VkImageView font_view;
        VkSampler font_sampler;

        vk_buffer_t vertex_buffer;
        vk_buffer_t index_buffer;

        const gui_t* gui;
        const vulkan_t* vulkan;

        error_t setup_font_texture();

    public:

        void bind_gui(const gui_t& gui);
        void bind_vulkan(const vulkan_t& vulkan);

        error_t init();

        ~gui_renderer_t();
    };
}

#endif // GUI_RENDERER_HPP
