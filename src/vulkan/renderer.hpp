#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <common.hpp>
#include <scene/scene.hpp>

#include "vulkan.hpp"

namespace bpmap
{

    class renderer_t
    {
        const vulkan_t* vulkan;
        const scene_t* scene;

        VkShaderModule raytrace;

        vk_image_t render_output;

        vk_fence_t render_finished;

        bool_t busy = false;

        static constexpr uint32_t pipeline_count = 1;
        static constexpr uint32_t raytrace_pipeline = 0;

        darray_t<VkPipeline> compute_pipelines;
        darray_t<VkPipelineLayout> compute_pipeline_layouts;

        VkDescriptorSetLayout descriptor_set_layout;
        VkDescriptorPool descriptor_pool;
        VkDescriptorSet descriptor_set;

        VkCommandBuffer command_buffer;
        vk_command_pool_t command_pool;

        error_t create_shaders();

        error_t create_descriptor_sets_layout();
        error_t create_descriptor_sets();
        error_t create_descriptor_pools();
        error_t create_compute_pipeline_layouts();
        error_t create_compute_pipelines();
        error_t create_command_pool();
        error_t create_command_buffers();
        error_t create_synchronization_primitives();
        error_t create_image();

    public:
        static constexpr const char* raytrace_kernel_path = "raytrace.comp.spv";

        void bind_vulkan(const vulkan_t&);
        void bind_scene(const scene_t&);

        error_t init();

        bool_t is_not_busy();

        error_t build_command_buffers();
        error_t submit_command_buffers();

        VkImageView render_output_view;
        VkSampler render_output_sampler;

        ~renderer_t();
    };

}

#endif // RENDERER_HPP
