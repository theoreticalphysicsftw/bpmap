#ifndef ERROR_HPP
#define ERROR_HPP

#include <common.hpp>

namespace bpmap
{
    enum class error_t
    {
        success,
        window_creation_fail,
        instance_creation_fail,
        device_search_fail,
        logical_device_creation_fail,
        graphics_queue_search_fail,
        copy_queue_search_fail,
        compute_queue_search_fail,
        queue_search_fail,
        queue_wait_fail,
        queue_submit_fail,
        get_queue_fail,
        command_pool_creation_fail,
        command_buffers_creation_fail,
        surface_creation_fail,
        surface_validation_fail,
        swapchain_creation_fail,
        allocator_creation_fail,
        buffer_creation_fail,
        semaphore_creation_fail,
        image_creation_fail,
        image_view_creation_fail,
        sampler_creation_fail,
        pipeline_creation_fail,
        render_pass_creation_fail,
        framebuffer_creation_fail,
        pipeline_layout_creation_fail,
        descriptor_set_layout_creation_fail,
        descriptor_pool_creation_fail,
        descriptor_set_allocation_fail,
        shader_creation_fail,
        vertex_shader_read_fail,
        fragment_shader_read_fail,
        font_texture_setup_fail,
        memory_mapping_fail,
        command_buffer_begin_fail,
    };

    string_t get_error_message(error_t e);
}

#endif // ERROR_HPP
