#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <common.hpp>
#include <scene/scene.hpp>

#include "vulkan.hpp"


namespace bpmap
{

    class renderer_t
    {
        const vk::device_t* vulkan;
        const scene_t* scene;
        vk::shader_registry_t* shader_registry;
        vk::sampler_registry_t* sampler_registry;

        vk::image_t render_output;

        vk::fence_t render_finished;
        vk::fence_t tmp_fence;

        bool_t busy = false;

        static constexpr uint32_t pipeline_count = 1;
        static constexpr uint32_t raytrace_pipeline = 0;


        darray_t<VkPipeline> compute_pipelines;
        darray_t<VkPipelineLayout> compute_pipeline_layouts;

        VkCommandBuffer command_buffer;
        vk::command_pool_t command_pool;


        vk::buffer_t triangles;

        vk::buffer_t vertices;
        vk::buffer_t normals;
        vk::buffer_t texcoords;

        vk::buffer_t materials;

        vk::buffer_t lights;

        vk::buffer_t scene_settings;

        error_t create_shaders();

        error_t create_buffers();
        error_t create_compute_pipeline_layouts();
        error_t create_compute_pipelines();
        error_t create_command_pool();
        error_t create_command_buffers();
        error_t create_synchronization_primitives();
        error_t create_image();

        template <typename T>
        error_t create_and_upload_buffer(vk::buffer_t& buffer, const T& data, vk::buffer_t& staging_buffer);


    public:
        static constexpr const char* raytrace_cs_name = "raytrace.comp.spv";

        renderer_t(
                    const vk::device_t& vulkan,
                    const scene_t& scene,
                    vk::shader_registry_t& shr,
                    vk::sampler_registry_t& sr
                  );

        error_t init();

        bool_t is_not_busy();

        error_t build_command_buffers();
        error_t submit_command_buffers();

        ~renderer_t();

        const vk::image_t& get_output() const { return render_output; }
    };

}

#endif // RENDERER_HPP
