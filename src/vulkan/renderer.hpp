#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <common.hpp>
#include "vulkan.hpp"

namespace bpmap
{

    class renderer_t
    {
        const vulkan_t* vulkan;

        VkShaderModule raytrace;

        VkImage render_output;
        VkImageView render_output_view;
        VkSampler render_output_sampler;

        static constexpr uint32_t pipeline_count = 1;
        static constexpr uint32_t raytrace_pipeline = 0;

        darray_t<VkPipeline> compute_pipelines;
        VkPipelineLayout compute_pipeline_layout;

        VkDescriptorSetLayout descriptor_set_layout;
        VkDescriptorPool descriptor_pool;
        VkDescriptorSet descriptor_set;


        error_t create_shaders();

        error_t create_descriptor_sets_layout();
        error_t create_descriptor_sets();
        error_t create_descriptor_pools();
        error_t create_compute_pipeline_layout();
        error_t create_compute_pipelines();

    public:
        static constexpr const char* raytrace_kernel_path = "raytrace.comp.spv";

        void bind_vulkan(const vulkan_t&);

        error_t init();

        ~renderer_t();
    };

}

#endif // RENDERER_HPP
