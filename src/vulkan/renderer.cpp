#include <algorithm>
#include <limits>

#include <core/io.hpp>

#include "renderer.hpp"

namespace bpmap
{

    void renderer_t::bind_vulkan(const vulkan_t& vk)
    {
        vulkan = &vk;
    }

    void renderer_t::bind_scene(const scene_t& s)
    {
        scene = &s;
    }

    error_t renderer_t::init()
    {
        auto status = create_shaders();

        if(status != error_t::success)
        {
            return status;
        }

        status = create_descriptor_pools();

        if(status != error_t::success)
        {
            return status;
        }

        status = create_descriptor_sets_layout();

        if(status != error_t::success)
        {
            return status;
        }

        status = create_descriptor_sets();

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

        status = update_descriptor_sets();

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


        if(vulkan->wait_for_fence(render_finished, 0) == error_t::success)
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

            auto status = vulkan->submit_work(submit_info, render_finished);

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
        return vulkan->create_fence(render_finished);
    }

    error_t renderer_t::create_image()
    {

        vk_image_desc_t desc;
        desc.width = scene->settings.resolution_x;
        desc.height = scene->settings.resolution_y;
        desc.format = vk_image_format_t::rgba32f;
        desc.tiling = vk_image_tiling_t::linear;
        desc.on_gpu = true;
        desc.usage = vk_usage_storage | vk_usage_sampled;
        
        if (vulkan->create_image(render_output, desc) != error_t::success)
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

        vulkan->create_fence(tmp_fence);

        if(vulkan->submit_work(submit_info, tmp_fence) != error_t::success)
        {
            return error_t::render_output_setup_fail;
        }

        auto timeout = std::numeric_limits<uint64_t>::max();

        vulkan->wait_for_fence(tmp_fence, timeout);

        // Create Sampler.
        VkSamplerCreateInfo sci = {};
        sci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sci.pNext = nullptr;
        sci.flags = 0;
        sci.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sci.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sci.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sci.anisotropyEnable = VK_FALSE;
        sci.maxAnisotropy = 1;
        sci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sci.mipLodBias = 0.0;
        sci.compareEnable = VK_FALSE;
        sci.minFilter = VK_FILTER_LINEAR;
        sci.magFilter = VK_FILTER_LINEAR;
        sci.minLod = 0.0;
        sci.maxLod = 0.0;
        sci.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

        if(vulkan->create_sampler(render_output_sampler, sci) != error_t::success)
        {
            return error_t::render_output_setup_fail;
        }

        return error_t::success;
    }

    renderer_t::~renderer_t()
    {
        vulkan->destroy_sampler(render_output_sampler);
        vulkan->destroy_descriptor_pool(descriptor_pool);
        vulkan->destroy_descriptor_set_layout(descriptor_set_layout);
        vulkan->destroy_pipeline_layout(compute_pipeline_layouts[raytrace_pipeline]);
        vulkan->destroy_pipeline(compute_pipelines[raytrace_pipeline]);
    }

    error_t renderer_t::create_shaders()
    {
        std::vector<uint8_t> raytrace_kernel_data;

        if(!read_whole_file(raytrace_kernel_path, raytrace_kernel_data))
        {
            return error_t::compute_kernel_read_fail;
        }

        return vulkan->create_shader(
                                       raytrace,
                                       (uint32_t*) raytrace_kernel_data.data(),
                                       raytrace_kernel_data.size(),
                                       shader_stage_t::compute
                                    );
    }

    error_t renderer_t::create_descriptor_sets_layout()
    {
        VkDescriptorSetLayoutBinding dslb[8] = {};

        dslb[0].binding = 0;
        dslb[0].descriptorCount = 1;
        dslb[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        dslb[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        dslb[1].binding = 1;
        dslb[1].descriptorCount = 1;
        dslb[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        dslb[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        dslb[2].binding = 2;
        dslb[2].descriptorCount = 1;
        dslb[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        dslb[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        dslb[3].binding = 3;
        dslb[3].descriptorCount = 1;
        dslb[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        dslb[3].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        dslb[4].binding = 4;
        dslb[4].descriptorCount = 1;
        dslb[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        dslb[4].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        dslb[5].binding = 5;
        dslb[5].descriptorCount = 1;
        dslb[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        dslb[5].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        dslb[6].binding = 6;
        dslb[6].descriptorCount = 1;
        dslb[6].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        dslb[6].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        dslb[7].binding = 7;
        dslb[7].descriptorCount = 1;
        dslb[7].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        dslb[7].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        VkDescriptorSetLayoutCreateInfo dslci = {};
        dslci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        dslci.pNext = nullptr;
        dslci.flags = 0;
        dslci.bindingCount = 8;
        dslci.pBindings = dslb;

        return vulkan->create_descriptor_set_layout(descriptor_set_layout, dslci);
    }

    error_t renderer_t::create_descriptor_sets()
    {
        VkDescriptorSetAllocateInfo dsai = {};
        dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        dsai.pNext = nullptr;
        dsai.descriptorSetCount = 1;
        dsai.pSetLayouts = &descriptor_set_layout;
        dsai.descriptorPool = descriptor_pool;

        return vulkan->allocate_descriptor_set(descriptor_set, dsai);
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
        vk_buffer_t staging_buffer;
        
        vk_buffer_desc_t staging_buffer_desc =
        {
            .size = staging_buffer_size,
            .usage = vk_buffer_usage_transfer_src,
            .on_gpu = false
        };

        if(vulkan->create_buffer(staging_buffer, staging_buffer_desc) != error_t::success)
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

        vk_buffer_desc_t scene_settings_buffer_desc =
        {
            .size = sizeof(scene_settings_t),
            .usage = vk_buffer_usage_uniform_buffer,
            .on_gpu = false
        };

        if(vulkan->create_buffer(scene_settings, scene_settings_buffer_desc) != error_t::success)
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
                                                  vk_buffer_t& buffer,
                                                  const T& data,
                                                  vk_buffer_t& staging_buffer
                                                 )
    {
        vk_buffer_desc_t desc =
        {
            .size = data.size() * sizeof(typename T::value_type),
            .usage = vk_buffer_usage_transfer_dst | vk_buffer_usage_storage_buffer,
            .on_gpu = true
        };

        if(vulkan->create_buffer(buffer, desc) != error_t::success)
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

        vk_fence_t fence;

        status = vulkan->create_fence(fence);

        if(status != error_t::success)
        {
            return status;
        }

        auto timeout = std::numeric_limits<uint64_t>::max();

        vulkan->submit_work(submit_info, fence);

        vulkan->wait_for_fence(fence, timeout);

        return error_t::success;
    }

    error_t renderer_t::update_descriptor_sets()
    {
        VkWriteDescriptorSet wds[8] = {};

        VkDescriptorBufferInfo scene_settings_info;
        scene_settings_info.buffer = scene_settings.get_handle();
        scene_settings_info.offset = 0;
        scene_settings_info.range = scene_settings.get_size();

        wds[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        wds[0].pNext = nullptr;
        wds[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        wds[0].descriptorCount = 1;
        wds[0].dstSet = descriptor_set;
        wds[0].dstBinding = 0;
        wds[0].dstArrayElement = 0;
        wds[0].pBufferInfo = &scene_settings_info;
        wds[0].pImageInfo = nullptr;
        wds[0].pTexelBufferView = nullptr;

        VkDescriptorImageInfo render_output_info;
        render_output_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        render_output_info.sampler = render_output_sampler;
        render_output_info.imageView = render_output.get_view();

        wds[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        wds[1].pNext = nullptr;
        wds[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        wds[1].descriptorCount = 1;
        wds[1].dstSet = descriptor_set;
        wds[1].dstBinding = 1;
        wds[1].dstArrayElement = 0;
        wds[1].pBufferInfo = nullptr;
        wds[1].pImageInfo = &render_output_info;
        wds[1].pTexelBufferView = nullptr;

        VkDescriptorBufferInfo triangles_info;
        triangles_info.buffer = triangles.get_handle();
        triangles_info.offset = 0;
        triangles_info.range = triangles.get_size();

        wds[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        wds[2].pNext = nullptr;
        wds[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        wds[2].descriptorCount = 1;
        wds[2].dstSet = descriptor_set;
        wds[2].dstBinding = 2;
        wds[2].dstArrayElement = 0;
        wds[2].pBufferInfo = &triangles_info;
        wds[2].pImageInfo = nullptr;
        wds[2].pTexelBufferView = nullptr;

        VkDescriptorBufferInfo vertices_info;
        vertices_info.buffer = vertices.get_handle();
        vertices_info.offset = 0;
        vertices_info.range = vertices.get_size();

        wds[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        wds[3].pNext = nullptr;
        wds[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        wds[3].descriptorCount = 1;
        wds[3].dstSet = descriptor_set;
        wds[3].dstBinding = 3;
        wds[3].dstArrayElement = 0;
        wds[3].pBufferInfo = &vertices_info;
        wds[3].pImageInfo = nullptr;
        wds[3].pTexelBufferView = nullptr;

        VkDescriptorBufferInfo normals_info;
        normals_info.buffer = normals.get_handle();
        normals_info.offset = 0;
        normals_info.range = normals.get_size();

        wds[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        wds[4].pNext = nullptr;
        wds[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        wds[4].descriptorCount = 1;
        wds[4].dstSet = descriptor_set;
        wds[4].dstBinding = 4;
        wds[4].dstArrayElement = 0;
        wds[4].pBufferInfo = &normals_info;
        wds[4].pImageInfo = nullptr;
        wds[4].pTexelBufferView = nullptr;

        VkDescriptorBufferInfo texcoords_info;
        texcoords_info.buffer = texcoords.get_handle();
        texcoords_info.offset = 0;
        texcoords_info.range = texcoords.get_size();

        wds[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        wds[5].pNext = nullptr;
        wds[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        wds[5].descriptorCount = 1;
        wds[5].dstSet = descriptor_set;
        wds[5].dstBinding = 5;
        wds[5].dstArrayElement = 0;
        wds[5].pBufferInfo = &texcoords_info;
        wds[5].pImageInfo = nullptr;
        wds[5].pTexelBufferView = nullptr;

        VkDescriptorBufferInfo materials_info;
        materials_info.buffer = materials.get_handle();
        materials_info.offset = 0;
        materials_info.range = materials.get_size();

        wds[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        wds[6].pNext = nullptr;
        wds[6].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        wds[6].descriptorCount = 1;
        wds[6].dstSet = descriptor_set;
        wds[6].dstBinding = 6;
        wds[6].dstArrayElement = 0;
        wds[6].pBufferInfo = &materials_info;
        wds[6].pImageInfo = nullptr;
        wds[6].pTexelBufferView = nullptr;

        VkDescriptorBufferInfo lights_info;
        lights_info.buffer = lights.get_handle();
        lights_info.offset = 0;
        lights_info.range = lights.get_size();

        wds[7].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        wds[7].pNext = nullptr;
        wds[7].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        wds[7].descriptorCount = 1;
        wds[7].dstSet = descriptor_set;
        wds[7].dstBinding = 7;
        wds[7].dstArrayElement = 0;
        wds[7].pBufferInfo = &lights_info;
        wds[7].pImageInfo = nullptr;
        wds[7].pTexelBufferView = nullptr;

        vulkan->update_descriptor_sets(wds, 8);

        return error_t::success;
    }

    error_t renderer_t::create_descriptor_pools()
    {
        VkDescriptorPoolSize pool_sizes[4] = {};

        pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        pool_sizes[0].descriptorCount = 1;
        pool_sizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        pool_sizes[1].descriptorCount = 1;
        pool_sizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        pool_sizes[2].descriptorCount = 6;

        VkDescriptorPoolCreateInfo dpci = {};
        dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        dpci.pNext = nullptr;
        dpci.maxSets = 1;
        dpci.poolSizeCount = 3;
        dpci.pPoolSizes = pool_sizes;

        return vulkan->create_descriptor_pool(descriptor_pool, dpci);
    }

    error_t renderer_t::create_compute_pipeline_layouts()
    {
        compute_pipeline_layouts.resize(pipeline_count);

        VkPipelineLayoutCreateInfo plci = {};
        plci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        plci.pNext = nullptr;
        plci.flags = 0;
        plci.pushConstantRangeCount = 0;
        plci.pPushConstantRanges = nullptr;
        plci.setLayoutCount = 1;
        plci.pSetLayouts = &descriptor_set_layout;

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
        pssci[raytrace_pipeline].module = raytrace.get_handle();
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
