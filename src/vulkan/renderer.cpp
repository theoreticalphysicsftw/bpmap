#include <algorithm>
#include <limits>

#include <core/io.hpp>

#include "renderer.hpp"

namespace bpmap
{
    renderer_t::renderer_t(
                            const vk::device_t& vulkan,
                            const scene_t& scene,
                            vk::shader_registry_t& shr,
                            vk::sampler_registry_t& sr
                          ) :
        vulkan(&vulkan), scene(&scene), shader_registry(&shr), sampler_registry(&sr)
    {

    }

    error_t renderer_t::init()
    {
        auto status = create_shaders();

        if(status != error_t::success)
        {
            return status;
        }

        status = create_compute_pipeline_layouts();

        if(status != error_t::success)
        {
            return status;
        }

        status = create_compute_pipelines();

        if(status != error_t::success)
        {
            return status;
        }

        status = create_command_pool();

        if(status != error_t::success)
        {
            return status;
        }

        status = create_command_buffers();

        if(status != error_t::success)
        {
            return status;
        }

        status = create_synchronization_primitives();

        if(status != error_t::success)
        {
            return status;
        }

        status = create_image();

        if(status != error_t::success)
        {
            return status;
        }

        status = create_buffers();

        if(status != error_t::success)
        {
            return status;
        }

        return error_t::success;
    }

    bool_t renderer_t::is_not_busy()
    {
        if(!busy)
        {
            return true;
        }


        if(render_finished.wait() == error_t::success)
        {
            busy = false;
        }

        return !busy;
    }

    error_t renderer_t::build_command_buffers()
    {
        VkCommandBufferBeginInfo cbbi = {};
        cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cbbi.pNext = nullptr;
        cbbi.pInheritanceInfo = nullptr;
        cbbi.flags = 0;

        vkResetCommandBuffer(command_buffer, 0);

        if(vkBeginCommandBuffer(command_buffer, &cbbi) != VK_SUCCESS)
        {
            return error_t::command_buffer_begin_fail;
        }

        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,compute_pipelines[raytrace_pipeline]);

        auto descriptor_set = vulkan->get_bindless_set();
        vkCmdBindDescriptorSets(
                                  command_buffer,
                                  VK_PIPELINE_BIND_POINT_COMPUTE,
                                  compute_pipeline_layouts[raytrace_pipeline],
                                  0,
                                  1,
                                  &descriptor_set,
                                  0,
                                  nullptr
                                );

        static constexpr uint32_t local_group_size_x = 8;
        static constexpr uint32_t local_group_size_y = 8;


        darray_t<uint32_t> slots;
        slots.push_back(vertices.get_slot());
        slots.push_back(normals.get_slot());
        slots.push_back(texcoords.get_slot());
        slots.push_back(materials.get_slot());
        slots.push_back(triangles.get_slot());
        slots.push_back(lights.get_slot());
        slots.push_back(scene_settings.get_slot());
        slots.push_back(render_output.get_slot());

        vkCmdPushConstants(
                            command_buffer,
                            compute_pipeline_layouts[raytrace_pipeline],
                            VK_SHADER_STAGE_ALL,
                            0,
                            slots.size() * 4,
                            slots.data()
                          );

        vkCmdDispatch(
                       command_buffer,scene->settings.resolution_x / local_group_size_x,
                       scene->settings.resolution_y / local_group_size_y,
                       1
                     );

        vkEndCommandBuffer(command_buffer);

        return error_t::success;
    }

    error_t renderer_t::submit_command_buffers()
    {
        if(is_not_busy())
        {
            VkSubmitInfo submit_info = {};
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.pNext = nullptr;
            submit_info.waitSemaphoreCount = 0;
            submit_info.pWaitSemaphores = nullptr;
            submit_info.pWaitDstStageMask = nullptr;
            submit_info.signalSemaphoreCount = 0;
            submit_info.pSignalSemaphores = nullptr;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &command_buffer;

            auto status = vulkan->submit_work(submit_info, &render_finished);

            if(status != error_t::success)
            {
                return status;
            }

            busy = true;
        }

        return error_t::success;
    }

    error_t renderer_t::create_command_buffers()
    {
        VkCommandBufferAllocateInfo cbai = {};
        cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cbai.pNext = nullptr;
        cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cbai.commandPool = command_pool.pool;
        cbai.commandBufferCount = 1;
        return vulkan->create_command_buffers(&command_buffer, cbai);
    }

    error_t renderer_t::create_synchronization_primitives()
    {
        return render_finished.create(*vulkan);
    }

    error_t renderer_t::create_image()
    {

        vk::image_desc_t desc;
        desc.width = scene->settings.resolution_x;
        desc.height = scene->settings.resolution_y;
        desc.format = vk::image_format_t::rgba32f;
        desc.tiling = vk::image_tiling_t::linear;
        desc.on_gpu = true;
        desc.usage = vk::usage_storage | vk::usage_sampled;
        
        if (render_output.create(*vulkan, desc) != error_t::success)
        {
            return error_t::render_output_setup_fail;
        }

        VkCommandBuffer tmp_buffer;

        VkCommandBufferAllocateInfo cbai = {};
        cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cbai.pNext = nullptr;
        cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cbai.commandPool = command_pool.pool;
        cbai.commandBufferCount = 1;

        vulkan->create_command_buffers(&tmp_buffer, cbai);

        VkCommandBufferBeginInfo cbbi = {};
        cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cbbi.pNext = nullptr;
        cbbi.pInheritanceInfo = nullptr;
        cbbi.flags = 0;

        if(vkBeginCommandBuffer(tmp_buffer, &cbbi))
        {
            return error_t::render_output_setup_fail;
        }

        VkImageSubresourceRange isr = {};
        isr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        isr.baseArrayLayer = 0;
        isr.baseMipLevel = 0;
        isr.layerCount = 1;
        isr.levelCount = 1;

        VkImageMemoryBarrier layout_barrier = {};
        layout_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        layout_barrier.pNext = nullptr;
        layout_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        layout_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        layout_barrier.image = render_output.get_image();
        layout_barrier.srcAccessMask = 0;
        layout_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        layout_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        layout_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        layout_barrier.subresourceRange = isr;

        vkCmdPipelineBarrier(
                              tmp_buffer,
                              VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                              VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                              0,
                              0,
                              nullptr,
                              0,
                              nullptr,
                              1,
                              &layout_barrier
                            );

        vkEndCommandBuffer(tmp_buffer);

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pNext = nullptr;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &tmp_buffer;
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = nullptr;
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitDstStageMask = nullptr;
        submit_info.pWaitSemaphores = nullptr;

        tmp_fence.create(*vulkan);

        if(vulkan->submit_work(submit_info, &tmp_fence) != error_t::success)
        {
            return error_t::render_output_setup_fail;
        }

        auto timeout = std::numeric_limits<uint64_t>::max();

        tmp_fence.wait(timeout);

        return error_t::success;
    }

    renderer_t::~renderer_t()
    {
        vulkan->destroy_pipeline_layout(compute_pipeline_layouts[raytrace_pipeline]);
        vulkan->destroy_pipeline(compute_pipelines[raytrace_pipeline]);
    }

    error_t renderer_t::create_shaders()
    {
        return shader_registry->add_from_file(
                                               raytrace_cs_name,
                                               vk::shader_stage_t::compute
                                             );
    }


    error_t renderer_t::create_buffers()
    {
        auto staging_buffer_size = std::max({
                                                scene->vertices.size() * sizeof(decltype(scene->vertices)::value_type),
                                                scene->normals.size() * sizeof(decltype(scene->normals)::value_type),
                                                scene->texcoords.size() * sizeof(decltype(scene->texcoords)::value_type),
                                                scene->materials.size() * sizeof(decltype(scene->materials)::value_type),
                                                scene->triangles.size() * sizeof(decltype(scene->triangles)::value_type),
                                                scene->lights.size() * sizeof(decltype(scene->lights)::value_type),
                                                scene->objects.size() *  sizeof(decltype(scene->objects)::value_type)
                                            });
        vk::buffer_t staging_buffer;
        
        vk::buffer_desc_t staging_buffer_desc =
        {
            .size = staging_buffer_size,
            .usage = vk::buffer_usage_transfer_src,
            .on_gpu = false,
            .dont_bind = true,
        };

        if(staging_buffer.create(*vulkan, staging_buffer_desc) != error_t::success)
        {
            return error_t::buffer_creation_fail;
        }

        auto status = create_and_upload_buffer(vertices, scene->vertices, staging_buffer);

        if(status != error_t::success)
        {
            return status;
        }

        status = create_and_upload_buffer(normals, scene->normals, staging_buffer);

        if(status != error_t::success)
        {
            return status;
        }

        status = create_and_upload_buffer(texcoords, scene->texcoords, staging_buffer);

        if(status != error_t::success)
        {
            return status;
        }

        status = create_and_upload_buffer(lights, scene->lights, staging_buffer);

        if(status != error_t::success)
        {
            return status;
        }

        status = create_and_upload_buffer(materials, scene->materials, staging_buffer);

        if(status != error_t::success)
        {
            return status;
        }

        status = create_and_upload_buffer(triangles, scene->triangles, staging_buffer);

        if(status != error_t::success)
        {
            return status;
        }

        vk::buffer_desc_t scene_settings_buffer_desc =
        {
            .size = sizeof(scene_settings_t),
            .usage = vk::buffer_usage_storage_buffer,
            .on_gpu = false
        };

        if(scene_settings.create(*vulkan, scene_settings_buffer_desc) != error_t::success)
        {
            return status;
        }

        void* mapped_settings;
        status = scene_settings.map(&mapped_settings);
        if(status != error_t::success)
        {
            return status;
        }

        memcpy(mapped_settings, &scene->settings, sizeof(scene->settings));
        scene_settings.unmap();

        return error_t::success;
    }

    template<typename T>
    error_t renderer_t::create_and_upload_buffer(
                                                  vk::buffer_t& buffer,
                                                  const T& data,
                                                  vk::buffer_t& staging_buffer
                                                 )
    {
        vk::buffer_desc_t desc =
        {
            .size = data.size() * sizeof(typename T::value_type),
            .usage = vk::buffer_usage_transfer_dst | vk::buffer_usage_storage_buffer,
            .on_gpu = true
        };

        if(buffer.create(*vulkan, desc) != error_t::success)
        {
            return error_t::buffer_creation_fail;
        }

        void* mapped;
        auto status = staging_buffer.map(&mapped);

        if(status != error_t::success)
        {
            return status;
        }

        memcpy(mapped, data.data(), desc.size);

        staging_buffer.unmap();

        VkCommandBuffer tmp_cmd_buffer;

        VkCommandBufferAllocateInfo cbai = {};
        cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cbai.pNext = nullptr;
        cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cbai.commandPool = command_pool.pool;
        cbai.commandBufferCount = 1;

        status = vulkan->create_command_buffers(&tmp_cmd_buffer, cbai);

        if(status != error_t::success)
        {
            return status;
        }

        VkCommandBufferBeginInfo cbbi = {};
        cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cbbi.pNext = nullptr;
        cbbi.pInheritanceInfo = nullptr;
        cbbi.flags = 0;

        if(vkBeginCommandBuffer(tmp_cmd_buffer, &cbbi) != VK_SUCCESS)
        {
            return error_t::buffer_creation_fail;
        }

        VkBufferCopy regions;
        regions.srcOffset = 0;
        regions.dstOffset = 0;
        regions.size = desc.size;

        vkCmdCopyBuffer(tmp_cmd_buffer, staging_buffer.get_handle(), buffer.get_handle(), 1, &regions);

        vkEndCommandBuffer(tmp_cmd_buffer);

        VkSubmitInfo submit_info;
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pNext = nullptr;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &tmp_cmd_buffer;
        submit_info.signalSemaphoreCount = 0;
        submit_info.waitSemaphoreCount = 0;

        vk::fence_t fence;

        status = fence.create(*vulkan);

        if(status != error_t::success)
        {
            return status;
        }

        auto timeout = std::numeric_limits<uint64_t>::max();

        vulkan->submit_work(submit_info, &fence);

        fence.wait(timeout);

        return error_t::success;
    }


    error_t renderer_t::create_compute_pipeline_layouts()
    {
        compute_pipeline_layouts.resize(pipeline_count);
    
        auto push_range = vulkan->get_push_range();
        auto layout = vulkan->get_bindless_layout();
        VkPipelineLayoutCreateInfo plci = {};
        plci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        plci.pNext = nullptr;
        plci.flags = 0;
        plci.pushConstantRangeCount = 1;
        plci.pPushConstantRanges = &push_range;
        plci.setLayoutCount = 1;
        plci.pSetLayouts = &layout;

        return vulkan->create_pipeline_layout(compute_pipeline_layouts[raytrace_pipeline], plci);
    }

    error_t renderer_t::create_compute_pipelines()
    {
        compute_pipelines.resize(pipeline_count);

        darray_t<VkPipelineShaderStageCreateInfo> pssci;
        pssci.resize(pipeline_count);

        pssci[raytrace_pipeline].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pssci[raytrace_pipeline].pNext = nullptr;
        pssci[raytrace_pipeline].stage = VK_SHADER_STAGE_COMPUTE_BIT;
        pssci[raytrace_pipeline].module = shader_registry->get(raytrace_cs_name).get_handle();
        pssci[raytrace_pipeline].pName = "main";
        pssci[raytrace_pipeline].pSpecializationInfo = nullptr;

        darray_t<VkComputePipelineCreateInfo> cpci;
        cpci.resize(pipeline_count);

        cpci[raytrace_pipeline].sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        cpci[raytrace_pipeline].pNext = nullptr;
        cpci[raytrace_pipeline].flags = 0;
        cpci[raytrace_pipeline].stage = pssci[raytrace_pipeline];
        cpci[raytrace_pipeline].layout = compute_pipeline_layouts[raytrace_pipeline];
        cpci[raytrace_pipeline].basePipelineHandle = VK_NULL_HANDLE;
        cpci[raytrace_pipeline].basePipelineIndex = 0;

        return vulkan->create_compute_pipelines(compute_pipelines, cpci);
    }

    error_t renderer_t::create_command_pool()
    {
        return vulkan->create_command_pool(command_pool);
    }
}
