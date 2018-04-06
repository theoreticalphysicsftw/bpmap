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

#include "core/algebra.hpp"
#include "core/io.hpp"
#include "vulkan.hpp"
#include "gui/gui.hpp"

namespace bpmap
{
    class gui_renderer_t
    {
        VkPipeline pipeline;
        VkDescriptorSetLayout descriptor_set_layout;
        VkPipelineLayout pipeline_layout;
        VkRenderPass render_pass;
        darray_t<VkFramebuffer> framebuffers;

        VkShaderModule vertex_shader;
        VkShaderModule fragment_shader;

        vk_image_t font_image;

        VkImageView font_view;
        VkSampler font_sampler;

        vk_buffer_t vertex_buffer;
        vk_buffer_t index_buffer;

        const gui_t* gui;
        const vulkan_t* vulkan;

        error_t setup_font_texture();
        error_t create_descriptor_sets_layout();
        error_t create_pipeline_layout();
        error_t create_pipeline();
        error_t create_renderpass();
        error_t create_framebuffer();
        error_t create_shaders();

    public:
        static constexpr const char_t* vertex_shader_path = "gui.vert.spv";
        static constexpr const char_t* fragment_shader_path = "gui.frag.spv";

        void bind_gui(const gui_t& gui);
        void bind_vulkan(const vulkan_t& vulkan);

        error_t init();

        ~gui_renderer_t();
    };

    struct gui_vertex_t
    {
        point2d_t position;
        point2d_t uv; // Parametrization coordinates
        color4d_t color;
    };
}

#endif // GUI_RENDERER_HPP
