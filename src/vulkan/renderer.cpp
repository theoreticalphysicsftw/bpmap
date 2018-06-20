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

        vkCmdDispatch(command_buffer,scene->settings.resolution_x/16, scene->settings.resolution_y/16, 1);

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
        VkExtent3D extent = {};
        extent.height = scene->settings.resolution_y;
        extent.width = scene->settings.resolution_x;
        extent.depth = 1;

        VkImageCreateInfo ici = {};
        ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        ici.pNext = nullptr;
        ici.flags = 0;
        ici.imageType = VK_IMAGE_TYPE_2D;
        ici.arrayLayers = 1;
        ici.mipLevels = 1;
        ici.samples = VK_SAMPLE_COUNT_1_BIT;
        ici.format = VK_FORMAT_R8G8B8A8_UNORM;
        ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        ici.tiling = VK_IMAGE_TILING_OPTIMAL;
        ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        ici.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        ici.pQueueFamilyIndices = nullptr;
        ici.queueFamilyIndexCount = 0;
        ici.extent = extent;

        VmaAllocationCreateInfo aci = {};
        aci.pool = VK_NULL_HANDLE;
        aci.flags = 0;
        aci.preferredFlags = 0;
        aci.requiredFlags = 0;
        aci.pUserData = nullptr;
        aci.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        if(vulkan->create_image(render_output, ici, aci) != error_t::success)
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
        layout_barrier.image = render_output.image;
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

        vk_fence_t tmp_fence;
        vulkan->create_fence(tmp_fence);

        if(vulkan->submit_work(submit_info, tmp_fence) != error_t::success)
        {
            return error_t::render_output_setup_fail;
        }

        auto timeout = std::numeric_limits<uint64_t>::max();

        vulkan->wait_for_fence(tmp_fence, timeout);

        // Create View and Sampler.

        VkImageViewCreateInfo ivci = {};
        ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ivci.pNext = nullptr;
        ivci.flags = 0;
        ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
        ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivci.subresourceRange = isr;
        ivci.image = render_output.image;
        ivci.components = {
                            VK_COMPONENT_SWIZZLE_R,
                            VK_COMPONENT_SWIZZLE_G,
                            VK_COMPONENT_SWIZZLE_B,
                            VK_COMPONENT_SWIZZLE_A
                           };

        if(vulkan->create_image_view(render_output_view, ivci) != error_t::success)
        {
            return error_t::render_output_setup_fail;
        }

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
        vulkan->destroy_image_view(render_output_view);
        vulkan->destroy_sampler(render_output_sampler);
        vulkan->destroy_shader(raytrace);
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

        VkShaderModuleCreateInfo raytrace_smci = {};
        raytrace_smci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        raytrace_smci.pNext = nullptr;
        raytrace_smci.flags = 0;
        raytrace_smci.codeSize = raytrace_kernel_data.size();
        raytrace_smci.pCode = (uint32_t*) raytrace_kernel_data.data();

        return vulkan->create_shader(raytrace, raytrace_smci);
    }

    error_t renderer_t::create_descriptor_sets_layout()
    {
        VkDescriptorSetLayoutBinding dslb[3] = {};

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

        VkDescriptorSetLayoutCreateInfo dslci = {};
        dslci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        dslci.pNext = nullptr;
        dslci.flags = 0;
        dslci.bindingCount = 3;
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

    error_t renderer_t::create_descriptor_pools()
    {
        VkDescriptorPoolSize pool_sizes[4] = {};

        pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        pool_sizes[0].descriptorCount = 1;
        pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        pool_sizes[1].descriptorCount = 1;
        pool_sizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        pool_sizes[2].descriptorCount = 1;
        pool_sizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        pool_sizes[3].descriptorCount = 1;

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
        pssci[raytrace_pipeline].module = raytrace;
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
