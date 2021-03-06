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
#include "renderer.hpp"
#include "gui/gui.hpp"

namespace bpmap
{
    class gui_renderer_t
    {
        VkPipeline pipeline;
        VkPipeline render_output_pipeline;
        VkPipelineLayout pipeline_layout;
        VkRenderPass render_pass;
        VkDescriptorSetLayout descriptor_set_layout;
        VkDescriptorPool descriptor_pool;
        VkDescriptorSet descriptor_set;
        VkDescriptorSet render_output_descriptor_set;
        darray_t<VkFramebuffer> framebuffers;

        vk_command_pool_t command_pool;
        darray_t<VkCommandBuffer> command_buffers;

        vk_semaphore_t image_available;
        vk_semaphore_t render_finished;

        VkShaderModule render_output_vertex_shader;
        VkShaderModule render_output_fragment_shader;
        VkShaderModule vertex_shader;
        VkShaderModule fragment_shader;

        vk_image_t font_image;

        VkImageView font_view;
        VkSampler font_sampler;

        vk_buffer_t vertex_buffer;
        vk_buffer_t index_buffer;
        vk_buffer_t gui_data_buffer;

        gui_data_t gui_data;

        gui_t* gui;
        const vulkan_t* vulkan;
        const renderer_t* renderer;

        error_t setup_font_texture();
        error_t create_descriptor_sets_layout();
        error_t create_descriptor_pool();
        error_t create_descriptor_sets();
        error_t update_descriptor_sets();
        error_t create_pipeline_layout();
        error_t create_pipeline();
        error_t create_renderpass();
        error_t create_framebuffer();
        error_t create_shaders();
        error_t create_command_pool();
        error_t create_command_buffers();
        error_t create_semaphores();
        error_t allocate_buffers();
        error_t create_buffers();
        error_t upload_gui_data();

        error_t build_command_buffer(uint32_t index);
        error_t submit_command_buffer(uint32_t index);
        error_t present_on_screen(uint64_t index);

    public:
        static constexpr const char_t* vertex_shader_path = "gui.vert.spv";
        static constexpr const char_t* fragment_shader_path = "gui.frag.spv";
        static constexpr const char_t* render_output_vertex_shader_path = "render_output.vert.spv";
        static constexpr const char_t* render_output_fragment_shader_path = "render_output.frag.spv";

        static constexpr uint32_t max_gui_ibuffer_size = 1 << 16;
        static constexpr uint32_t max_gui_vbuffer_size = 1 << 20;

        void bind_gui(gui_t& gui);
        void bind_vulkan(const vulkan_t& vulkan);
        void bind_renderer(const renderer_t& renderer);

        error_t init();

        error_t render_frame();

        ~gui_renderer_t();
    };

}

#endif // GUI_RENDERER_HPP
