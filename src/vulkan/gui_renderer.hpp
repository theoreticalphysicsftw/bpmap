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

        vk::command_pool_t command_pool;
        darray_t<VkCommandBuffer> command_buffers;

        vk::semaphore_t image_available;
        vk::semaphore_t render_finished;

        vk::shader_t render_output_vertex_shader;
        vk::shader_t render_output_fragment_shader;
        vk::shader_t vertex_shader;
        vk::shader_t fragment_shader;

        vk::image_t font_image;
        const vk::sampler_t* font_sampler;
        const vk::sampler_t* ro_sampler;

        vk::buffer_t vertex_buffer;
        vk::buffer_t index_buffer;
        vk::buffer_t gui_data_buffer;

        gui_data_t gui_data;

        gui_t* gui;
        const vk::device_t* vulkan;
        const renderer_t* renderer;
        vk::shader_registry_t* shader_registry;
        vk::sampler_registry_t* sampler_registry;

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
        static constexpr const char_t* gui_vs_name = "gui.vert.spv";
        static constexpr const char_t* gui_fs_name = "gui.frag.spv";
        static constexpr const char_t* render_output_vs_name = "render_output.vert.spv";
        static constexpr const char_t* render_output_fs_name = "render_output.frag.spv";

        static constexpr uint32_t max_gui_ibuffer_size = 1 << 16;
        static constexpr uint32_t max_gui_vbuffer_size = 1 << 20;

        gui_renderer_t(
                        gui_t& gui, 
                        const vk::device_t& vk,
                        vk::shader_registry_t& shader_registry,
                        vk::sampler_registry_t& sampler_registry,
                        const renderer_t& r
                      );

        error_t init();

        error_t render_frame();

        ~gui_renderer_t();
    };

}

#endif // GUI_RENDERER_HPP
