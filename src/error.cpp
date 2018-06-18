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


#include "error.hpp"

namespace bpmap
{
    string_t get_error_message(error_t e)
    {
        switch(e)
        {
            case error_t::window_creation_fail :
                return "Failed to create window!";

            case error_t::instance_creation_fail :
                return "Failed to create vulkan instance!";

            case error_t::device_search_fail :
                return "Failed to find suitable physical device!";

            case error_t::logical_device_creation_fail:
                return "Failed to create logical device!";

            case error_t::get_queue_fail:
                return "Failed to get device queue!";

            case error_t::command_pool_creation_fail:
                return "Failed to create command pool!";

            case error_t::command_buffers_creation_fail:
                return "Failed to create command buffers!";

            case error_t::surface_creation_fail:
                return "Failed to create window surface!";

            case error_t::surface_validation_fail:
                return "Failed to validate window surface support!";

            case error_t::swapchain_creation_fail:
                return "Failed to create window swapchain!";

            case error_t::allocator_creation_fail:
                return "Failed to create device memory allocator!";

            case error_t::buffer_creation_fail:
                return "Failed to create buffer!";

            case error_t::semaphore_creation_fail:
                return "Failed to create semaphore!";

            case error_t::image_creation_fail:
                return "Failed to create image!";

            case error_t::image_view_creation_fail:
                return "Failed to create image view!";

            case error_t::sampler_creation_fail:
                return "Failed to create sampler!";

            case error_t::font_texture_setup_fail:
                return "Failed to setup font texture!";

            case error_t::pipeline_layout_creation_fail:
                return "Failed to create pipeline layout!";

            case error_t::descriptor_set_layout_creation_fail:
                return "Failed to create descriptor set layout!";

            case error_t::descriptor_pool_creation_fail:
                return "Failed to create descriptor pool!";

            case error_t::descriptor_set_allocation_fail:
                return "Failed to allocate descriptor set!";

            case error_t::pipeline_creation_fail:
                return "Failed to create pipeline!";

            case error_t::render_pass_creation_fail:
                return "Failed to create render pass!";

            case error_t::framebuffer_creation_fail:
                return "Failed to create framebuffer!";

            case error_t::shader_creation_fail:
                return "Failed to create shader!";

            case error_t::vertex_shader_read_fail:
                return "Failed to read vertex shader!";

            case error_t::compute_kernel_read_fail:
                return "Failed to read compute kernel!";

            case error_t::fragment_shader_read_fail:
                return "Failed to read fragment shader!";

            case error_t::graphics_queue_search_fail:
                return "Failed to find graphics queue!";

            case error_t::compute_queue_search_fail:
                return "Failed to find compute queue!";

            case error_t::queue_search_fail:
                return "Failed to find compute queue!";

            case error_t::queue_submit_fail:
                return "Failed to submit work to queue!";

            case error_t::queue_wait_fail:
                return "Failed to wait for queue to get idle!";

            case error_t::copy_queue_search_fail:
                return "Failed to find copy queue!";

            case error_t::memory_mapping_fail:
                return "Failed to map memory!";

            case error_t::command_buffer_begin_fail:
                return "Failed to begin command buffer!";

            case error_t::fence_creation_fail:
                return "Failed to create fence!";

            case error_t::device_lost:
                return "Lost connection to logical device!";

            case error_t::presentation_fail:
                return "Failed to present images on screen";

            default:
                return "Unknown error occured";
        }
    }
}
