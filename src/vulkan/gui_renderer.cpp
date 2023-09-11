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

#include <cstddef>
#include <limits>

#include "gui_renderer.hpp"


namespace bpmap
{
    gui_renderer_t::gui_renderer_t(
                                    gui_t& gui,
                                    const vk::device_t& vk,
                                    vk::shader_registry_t& shr,
                                    vk::sampler_registry_t& sr,
                                    const renderer_t& r
                                  ) :
        gui(&gui), vulkan(&vk), shader_registry(&shr), sampler_registry(&sr), renderer(&r)
    {

    }

    error_t gui_renderer_t::setup_font_texture()
    {
        VkExtent3D extent = {};
        extent.height = gui->get_font_height();
        extent.width = gui->get_font_width();
        extent.depth = 1;

        vk::image_desc_t desc;
        desc.width = gui->get_font_width();
        desc.height = gui->get_font_height(),
        desc.format = vk::image_format_t::rgba8u,
        desc.tiling = vk::image_tiling_t::optimal,
        desc.on_gpu = true,
        desc.usage = vk::usage_transfer_dst | vk::usage_sampled;

        if (font_image.create(*vulkan, desc) != error_t::success)
        {
            return error_t::font_texture_setup_fail;
        }

        vk::buffer_t staging_buffer;

        vk::buffer_desc_t staging_buffer_desc =
        {
            .size = size_t(gui->get_font_width() * gui->get_font_height() * 4),
            .usage = vk::buffer_usage_transfer_src,
            .on_gpu = false
        };

        if(staging_buffer.create(*vulkan, staging_buffer_desc) != error_t::success)
        {
            return error_t::font_texture_setup_fail;
        }

        void* mapped;
        auto status = staging_buffer.map(&mapped);

        if(status != error_t::success)
        {
            return status;
        }

        memcpy(mapped, gui->get_raw_font(), staging_buffer_desc.size);

        staging_buffer.unmap();

        VkCommandBuffer tmp_cmd_buffer;

        VkCommandBufferAllocateInfo cbai = {};
        cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cbai.pNext = nullptr;
        cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cbai.commandPool = command_pool.pool;
        cbai.commandBufferCount = 1;

        if(vulkan->create_command_buffers(&tmp_cmd_buffer, cbai) != error_t::success)
        {
            return error_t::font_texture_setup_fail;
        }

        VkCommandBufferBeginInfo cbbi = {};
        cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cbbi.pNext = nullptr;
        cbbi.pInheritanceInfo = nullptr;
        cbbi.flags = 0;

        if(vkBeginCommandBuffer(tmp_cmd_buffer, &cbbi) != VK_SUCCESS)
        {
            return error_t::font_texture_setup_fail;
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
        layout_barrier.image = font_image.get_image();
        layout_barrier.srcAccessMask = 0;
        layout_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        layout_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        layout_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        layout_barrier.subresourceRange = isr;

        vkCmdPipelineBarrier(
                              tmp_cmd_buffer,
                              VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                              VK_PIPELINE_STAGE_TRANSFER_BIT,
                              0,
                              0,
                              nullptr,
                              0,
                              nullptr,
                              1,
                              &layout_barrier
                            );

        VkImageSubresourceLayers isl = {};
        isl.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        isl.layerCount = 1;
        isl.baseArrayLayer = 0;
        isl.mipLevel = 0;

        VkBufferImageCopy bic = {};
        bic.bufferOffset = 0;
        bic.bufferRowLength = 0;
        bic.bufferImageHeight = 0;
        bic.imageSubresource = isl;
        bic.imageExtent = extent;
        bic.imageOffset = {0, 0, 0};

        vkCmdCopyBufferToImage(
                                tmp_cmd_buffer,
                                staging_buffer.get_handle(),
                                font_image.get_image(),
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                1,
                                &bic
                              );

        VkImageMemoryBarrier final_layout_barrier = {};
        final_layout_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        final_layout_barrier.pNext = nullptr;
        final_layout_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        final_layout_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        final_layout_barrier.image = font_image.get_image();
        final_layout_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        final_layout_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        final_layout_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        final_layout_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        final_layout_barrier.subresourceRange = isr;

        vkCmdPipelineBarrier(
                              tmp_cmd_buffer,
                              VK_PIPELINE_STAGE_TRANSFER_BIT,
                              VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                              0,
                              0,
                              nullptr,
                              0,
                              nullptr,
                              1,
                              &final_layout_barrier
                            );

        if(vkEndCommandBuffer(tmp_cmd_buffer) != VK_SUCCESS)
        {
            return error_t::font_texture_setup_fail;
        }

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pNext = nullptr;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &tmp_cmd_buffer;
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = nullptr;
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitDstStageMask = nullptr;
        submit_info.pWaitSemaphores = nullptr;

        if(vulkan->submit_work(submit_info, nullptr) != error_t::success)
        {
            return error_t::font_texture_setup_fail;
        }

        vulkan->wait_idle();

        gui->finalize_font_atlas();


        vk::sampler_desc_t font_sampler_desc;
        vk::sampler_desc_t ro_sampler_desc;
        status = sampler_registry->add(font_sampler_desc);

        if (status != error_t::success)
        {
            return status;
        }

        font_sampler = &sampler_registry->get(font_sampler_desc);

        status = sampler_registry->add(ro_sampler_desc);
        if (status != error_t::success)
        {
            return status;
        }

        ro_sampler = &sampler_registry->get(ro_sampler_desc);

        return error_t::success;
    }


    error_t gui_renderer_t::create_descriptor_sets_layout()
    {
        VkDescriptorSetLayoutBinding dslb[2] = {};

        dslb[0].binding = 0;
        dslb[0].descriptorCount = 1;
        dslb[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        dslb[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        dslb[1].binding = 1;
        dslb[1].descriptorCount = 1;
        dslb[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        dslb[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo dslci = {};
        dslci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        dslci.pNext = nullptr;
        dslci.flags = 0;
        dslci.bindingCount = 2;
        dslci.pBindings = dslb;

        return vulkan->create_descriptor_set_layout(descriptor_set_layout, dslci);
    }

    error_t gui_renderer_t::create_descriptor_pool()
    {
        VkDescriptorPoolSize pool_sizes[2] = {};

        pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        pool_sizes[0].descriptorCount = 2;
        pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        pool_sizes[1].descriptorCount = 2;

        VkDescriptorPoolCreateInfo dpci = {};
        dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        dpci.pNext = nullptr;
        dpci.maxSets = 2;
        dpci.poolSizeCount = 2;
        dpci.pPoolSizes = pool_sizes;

        return vulkan->create_descriptor_pool(descriptor_pool, dpci);
    }

    error_t gui_renderer_t::create_descriptor_sets()
    {
        VkDescriptorSetAllocateInfo dsai = {};
        dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        dsai.pNext = nullptr;
        dsai.descriptorSetCount = 1;
        dsai.pSetLayouts = &descriptor_set_layout;
        dsai.descriptorPool = descriptor_pool;

        auto status = vulkan->allocate_descriptor_set(descriptor_set, dsai);

        if(status != error_t::success)
        {
            return status;
        }

        return vulkan->allocate_descriptor_set(render_output_descriptor_set, dsai);
    }

    error_t gui_renderer_t::update_descriptor_sets()
    {
        VkDescriptorBufferInfo gui_data_bind_info = {};
        gui_data_bind_info.buffer = gui_data_buffer.get_handle();
        gui_data_bind_info.offset = 0;
        gui_data_bind_info.range = sizeof(gui_data);

        VkDescriptorImageInfo font_texture_bind_info = {};
        font_texture_bind_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        font_texture_bind_info.imageView = font_image.get_view();
        font_texture_bind_info.sampler = font_sampler->get_handle();

        VkDescriptorImageInfo render_output_bind_info = {};
        render_output_bind_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        render_output_bind_info.imageView = renderer->get_output().get_view();
        render_output_bind_info.sampler = ro_sampler->get_handle();

        VkWriteDescriptorSet wds[2] = {};

        wds[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        wds[0].pNext = nullptr;
        wds[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        wds[0].descriptorCount = 1;
        wds[0].dstSet = descriptor_set;
        wds[0].dstBinding = 0;
        wds[0].dstArrayElement = 0;
        wds[0].pBufferInfo = &gui_data_bind_info;
        wds[0].pImageInfo = nullptr;
        wds[0].pTexelBufferView = nullptr;

        wds[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        wds[1].pNext = nullptr;
        wds[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        wds[1].descriptorCount = 1;
        wds[1].dstSet = descriptor_set;
        wds[1].dstBinding = 1;
        wds[1].dstArrayElement = 0;
        wds[1].pBufferInfo = nullptr;
        wds[1].pImageInfo = &font_texture_bind_info;
        wds[1].pTexelBufferView = nullptr;

        vulkan->update_descriptor_sets(wds, 2);

        wds[0].dstSet = render_output_descriptor_set;
        wds[1].dstSet = render_output_descriptor_set;
        wds[1].pImageInfo = &render_output_bind_info;


        vulkan->update_descriptor_sets(wds, 2);

        return error_t::success;
    }


    error_t gui_renderer_t::create_pipeline_layout()
    {
        VkPipelineLayoutCreateInfo plci = {};
        plci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        plci.pNext = nullptr;
        plci.flags = 0;
        plci.pushConstantRangeCount = 0;
        plci.pPushConstantRanges = nullptr;
        plci.setLayoutCount = 1;
        plci.pSetLayouts = &descriptor_set_layout;

        return vulkan->create_pipeline_layout(pipeline_layout, plci);
    }


    error_t gui_renderer_t::create_pipeline()
    {
        VkPipelineShaderStageCreateInfo pssci[2] = {};

        pssci[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pssci[0].pNext = nullptr;
        pssci[0].pName = "main";
        pssci[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        pssci[0].module = shader_registry->get(gui_vs_name).get_handle();
        pssci[0].pSpecializationInfo = nullptr;

        pssci[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pssci[1].pNext = nullptr;
        pssci[1].pName = "main";
        pssci[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        pssci[1].module = shader_registry->get(gui_fs_name).get_handle();
        pssci[1].pSpecializationInfo = nullptr;

        VkPipelineRasterizationStateCreateInfo prsci = {};
        prsci.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        prsci.pNext = nullptr;
        prsci.flags = 0;
        prsci.depthClampEnable = VK_FALSE;
        prsci.depthBiasEnable = VK_FALSE;
        prsci.rasterizerDiscardEnable = VK_FALSE;
        prsci.lineWidth = 1.0;
        prsci.cullMode = VK_CULL_MODE_BACK_BIT;
        prsci.frontFace = VK_FRONT_FACE_CLOCKWISE;
        prsci.polygonMode = VK_POLYGON_MODE_FILL;

        VkPipelineMultisampleStateCreateInfo pmsci = {};
        pmsci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        pmsci.pNext = nullptr;
        pmsci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkVertexInputBindingDescription vibd = {};
        vibd.binding = 0;
        vibd.stride = sizeof(gui_vertex_t);
        vibd.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription viad[3] = {};

        viad[0].binding = 0;
        viad[0].location = 0;
        viad[0].offset = offsetof(gui_vertex_t, position);
        viad[0].format = VK_FORMAT_R32G32_SFLOAT;

        viad[1].binding = 0;
        viad[1].location = 1;
        viad[1].offset = offsetof(gui_vertex_t, uv);
        viad[1].format = VK_FORMAT_R32G32_SFLOAT;

        viad[2].binding = 0;
        viad[2].location = 2;
        viad[2].offset = offsetof(gui_vertex_t, color);
        viad[2].format = VK_FORMAT_R8G8B8A8_UINT;

        VkPipelineVertexInputStateCreateInfo pvisci = {};
        pvisci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        pvisci.pNext = nullptr;
        pvisci.vertexBindingDescriptionCount = 1;
        pvisci.pVertexBindingDescriptions = &vibd;
        pvisci.vertexAttributeDescriptionCount = 3;
        pvisci.pVertexAttributeDescriptions = viad;

        VkPipelineInputAssemblyStateCreateInfo piasci = {};
        piasci.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        piasci.pNext = nullptr;
        piasci.flags = 0;
        piasci.primitiveRestartEnable = VK_FALSE;
        piasci.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkPipelineViewportStateCreateInfo pvsci = {};
        pvsci.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        pvsci.pNext = nullptr;
        pvsci.flags = 0;
        pvsci.viewportCount = 1;
        pvsci.scissorCount = 1;

        VkPipelineDepthStencilStateCreateInfo pdssci = {};
        pdssci.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        pdssci.pNext = nullptr;
        pdssci.depthTestEnable = VK_FALSE;
        pdssci.depthWriteEnable = VK_FALSE;
        pdssci.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        pdssci.depthBoundsTestEnable = VK_FALSE;
        pdssci.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState pcbas;
        pcbas.blendEnable = VK_TRUE;
        pcbas.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        pcbas.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        pcbas.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        pcbas.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        pcbas.colorBlendOp = VK_BLEND_OP_ADD;
        pcbas.alphaBlendOp = VK_BLEND_OP_ADD;
        pcbas.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                               VK_COLOR_COMPONENT_B_BIT |
                               VK_COLOR_COMPONENT_G_BIT |
                               VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendStateCreateInfo pcbsci = {};
        pcbsci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        pcbsci.pNext = nullptr;
        pcbsci.flags = 0;
        pcbsci.logicOpEnable = VK_FALSE;
        pcbsci.attachmentCount = 1;
        pcbsci.pAttachments = &pcbas;

        VkDynamicState dynamic_states[2];
        dynamic_states[0] = VK_DYNAMIC_STATE_VIEWPORT;
        dynamic_states[1] = VK_DYNAMIC_STATE_SCISSOR;

        VkPipelineDynamicStateCreateInfo pdsci = {};
        pdsci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        pdsci.pNext = nullptr;
        pdsci.flags = 0;
        pdsci.dynamicStateCount = 2;
        pdsci.pDynamicStates = dynamic_states;

        VkGraphicsPipelineCreateInfo gpci = {};
        gpci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        gpci.pNext = nullptr;
        gpci.flags = 0;
        gpci.stageCount = 2;
        gpci.pStages = pssci;
        gpci.pRasterizationState = &prsci;
        gpci.pMultisampleState = &pmsci;
        gpci.pVertexInputState = &pvisci;
        gpci.pInputAssemblyState = &piasci;
        gpci.pViewportState = &pvsci;
        gpci.pDepthStencilState = &pdssci;
        gpci.pColorBlendState = &pcbsci;
        gpci.pDynamicState = &pdsci;
        gpci.renderPass = render_pass;
        gpci.subpass = 1;
        gpci.layout = pipeline_layout;

        auto status = vulkan->create_graphics_pipeline(pipeline, gpci);

        if(status != error_t::success)
        {
            return status;
        }

        pssci[0].module = shader_registry->get(render_output_vs_name).get_handle();
        pssci[1].module = shader_registry->get(render_output_fs_name).get_handle();
        gpci.subpass = 0;

        pvisci.vertexBindingDescriptionCount = 0;
        pvisci.vertexAttributeDescriptionCount = 0;

        return vulkan->create_graphics_pipeline(render_output_pipeline, gpci);
    }


    error_t gui_renderer_t::create_renderpass()
    {
        VkAttachmentDescription attachment_description = {};
        attachment_description.flags = 0;
        attachment_description.format = vulkan->get_swapchain_format();
        attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference attachment_reference = {};
        attachment_reference.attachment = 0;
        attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpasses[2];
        subpasses[0].flags = 0;
        subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpasses[0].inputAttachmentCount = 0;
        subpasses[0].pInputAttachments = nullptr;
        subpasses[0].preserveAttachmentCount = 0;
        subpasses[0].pPreserveAttachments = nullptr;
        subpasses[0].colorAttachmentCount = 1;
        subpasses[0].pColorAttachments = &attachment_reference;
        subpasses[0].pDepthStencilAttachment = nullptr;
        subpasses[0].pResolveAttachments = nullptr;

        subpasses[1].flags = 0;
        subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpasses[1].inputAttachmentCount = 0;
        subpasses[1].pInputAttachments = nullptr;
        subpasses[1].preserveAttachmentCount = 0;
        subpasses[1].pPreserveAttachments = nullptr;
        subpasses[1].colorAttachmentCount = 1;
        subpasses[1].pColorAttachments = &attachment_reference;
        subpasses[1].pDepthStencilAttachment = nullptr;
        subpasses[1].pResolveAttachments = nullptr;

        VkSubpassDependency dependencies = {};
        dependencies.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependencies.srcSubpass = 0;
        dependencies.dstSubpass = 1;
        dependencies.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

        VkRenderPassCreateInfo rpci = {};
        rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        rpci.pNext = nullptr;
        rpci.flags = 0;
        rpci.attachmentCount = 1;
        rpci.pAttachments = &attachment_description;
        rpci.subpassCount = 2;
        rpci.pSubpasses = subpasses;
        rpci.dependencyCount = 1;
        rpci.pDependencies = &dependencies;

        return vulkan->create_renederpass(render_pass, rpci);
    }


    error_t gui_renderer_t::create_framebuffer()
    {
        VkFramebufferCreateInfo fbci = {};
        fbci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbci.pNext = nullptr;
        fbci.flags = 0;
        fbci.renderPass = render_pass;

        return vulkan->create_framebuffers(framebuffers, fbci);
    }


    error_t gui_renderer_t::create_shaders()
    {
        darray_t<pair_t<string_t, vk::shader_stage_t>> shaders
        {
            {gui_vs_name, vk::shader_stage_t::vertex},
            {gui_fs_name, vk::shader_stage_t::fragment},
            {render_output_vs_name, vk::shader_stage_t::vertex},
            {render_output_fs_name, vk::shader_stage_t::fragment},
        };

        return shader_registry->add_from_file(shaders);
    }


    error_t gui_renderer_t::create_command_pool()
    {
        return vulkan->create_command_pool(command_pool);
    }

    error_t gui_renderer_t::create_command_buffers()
    {
        command_buffers.resize(framebuffers.size());

        VkCommandBufferAllocateInfo cbai = {};
        cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cbai.pNext = nullptr;
        cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cbai.commandPool = command_pool.pool;
        cbai.commandBufferCount = command_buffers.size();

        return vulkan->create_command_buffers(command_buffers.data(), cbai);
    }

    error_t gui_renderer_t::create_semaphores()
    {
        auto status = image_available.create(*vulkan);

        if(status != error_t::success)
        {
            return status;
        }

        return render_finished.create(*vulkan);
    }

    error_t gui_renderer_t::allocate_buffers()
    {

        vk::buffer_desc_t index_buffer_desc =
        {
            .size = max_gui_ibuffer_size,
            .usage = vk::buffer_usage_index_buffer,
            .on_gpu = false
        };

        auto status = index_buffer.create(*vulkan, index_buffer_desc);
        if(status != error_t::success)
        {
            return status;
        }

        vk::buffer_desc_t vertex_buffer_desc =
        {
            .size = max_gui_vbuffer_size,
            .usage = vk::buffer_usage_index_buffer,
            .on_gpu = false
        };

        status = vertex_buffer.create(*vulkan, vertex_buffer_desc);
        if(status != error_t::success)
        {
            return status;
        }

        vk::buffer_desc_t gui_data_buffer_desc =
        {
            .size = sizeof(gui_data),
            .usage = vk::buffer_usage_uniform_buffer,
            .on_gpu = false
        };
        status = gui_data_buffer.create(*vulkan, gui_data_buffer_desc);

        if(status != error_t::success)
        {
            return status;
        }

        return error_t::success;
    }

    error_t gui_renderer_t::create_buffers()
    {
        void* ibuffer;
        index_buffer.map(&ibuffer);

        void* vbuffer;
        vertex_buffer.map(&vbuffer);

        gui->emit_buffers(ibuffer, max_gui_ibuffer_size, vbuffer, max_gui_vbuffer_size);

        index_buffer.unmap();
        vertex_buffer.unmap();

        return error_t::success;
    }

    error_t gui_renderer_t::upload_gui_data()
    {
        void* data;

        gui_data = gui->get_gui_data();

        gui_data_buffer.map(&data);
        memcpy(data, &gui_data, sizeof(gui_data));
        gui_data_buffer.unmap();

        return error_t::success;
    }

    error_t gui_renderer_t::build_command_buffer(uint32_t index)
    {
        VkCommandBufferBeginInfo cbbi = {};
        cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cbbi.pNext = nullptr;
        cbbi.pInheritanceInfo = nullptr;
        cbbi.flags = 0;

        vkResetCommandBuffer(command_buffers[index], 0);

        if(vkBeginCommandBuffer(command_buffers[index], &cbbi) != VK_SUCCESS)
        {
            return error_t::command_buffer_begin_fail;
        }

        VkExtent2D extent;
        extent.height = gui->get_height();
        extent.width = gui->get_width();

        VkOffset2D offset = {0, 0};

        VkRect2D render_area;
        render_area.extent = extent;
        render_area.offset = offset;  

        // VkClearValue clear_values;
        // clear_values.color = {0.0,0.0,0.0,0.0};

        VkRenderPassBeginInfo rpbi = {};
        rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rpbi.pNext = nullptr;
        rpbi.renderPass = render_pass;
        rpbi.framebuffer = framebuffers[index];
        rpbi.clearValueCount = 0;
        rpbi.pClearValues = nullptr;
        rpbi.renderArea = render_area;

        VkViewport viewport;
        viewport.height = gui->get_height();
        viewport.width = gui->get_width();
        viewport.x = 0.0;
        viewport.y = 0.0;
        viewport.minDepth = 0.0;
        viewport.maxDepth = 1.0;


        vkCmdBeginRenderPass(command_buffers[index], &rpbi, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(command_buffers[index], VK_PIPELINE_BIND_POINT_GRAPHICS, render_output_pipeline);

        vkCmdBindDescriptorSets(
                                 command_buffers[index],
                                 VK_PIPELINE_BIND_POINT_GRAPHICS,
                                 pipeline_layout,
                                 0,
                                 1,
                                 &render_output_descriptor_set,
                                 0,
                                 nullptr
                                );

        vkCmdSetScissor(command_buffers[index], 0, 1, &render_area);
        vkCmdSetViewport(command_buffers[index], 0, 1, &viewport);

        vkCmdDraw(command_buffers[index], 6, 1, 0, 0);


        // Draw Gui

        vkCmdNextSubpass(command_buffers[index], VK_SUBPASS_CONTENTS_INLINE);

        vkCmdSetViewport(command_buffers[index], 0, 1, &viewport);

        vkCmdBindPipeline(command_buffers[index], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        vkCmdBindDescriptorSets(
                                 command_buffers[index],
                                 VK_PIPELINE_BIND_POINT_GRAPHICS,
                                 pipeline_layout,
                                 0,
                                 1,
                                 &descriptor_set,
                                 0,
                                 nullptr
                                );

        VkDeviceSize off = 0;
        VkBuffer vertex_buffer_handle = vertex_buffer.get_handle();
        vkCmdBindVertexBuffers(command_buffers[index], 0, 1, &vertex_buffer_handle, &off);
        vkCmdBindIndexBuffer(command_buffers[index], index_buffer.get_handle(), off, VK_INDEX_TYPE_UINT16);

        auto cmds = gui->emit_draw_calls();

        for(auto& cmd : cmds)
        {
            VkExtent2D extent;
            extent.width = cmd.scissor_width;
            extent.height = cmd.scissor_height;

            VkOffset2D offset;
            offset.x = cmd.scissor_horizontal_offset;
            offset.y = cmd.scissor_vertical_offset;

            VkRect2D scissor;
            scissor.extent = extent;
            scissor.offset = offset;

            vkCmdSetScissor(command_buffers[index], 0, 1, &scissor);

            vkCmdDrawIndexed(command_buffers[index], cmd.elements, 1, cmd.offset, 0, 0);
        }

        vkCmdEndRenderPass(command_buffers[index]);
        vkEndCommandBuffer(command_buffers[index]);

        return error_t::success;
    }

    error_t gui_renderer_t::submit_command_buffer(uint32_t index)
    {
        vk::fence_t done_rendering;
        done_rendering.create(*vulkan);

        VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        auto image_available_semaphore = image_available.get_handle();
        auto render_finished_semaphore = render_finished.get_handle();
        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pNext = nullptr;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &image_available_semaphore;
        submit_info.pWaitDstStageMask = wait_stages;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &render_finished_semaphore;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffers[index];

        static constexpr uint64_t timeout = std::numeric_limits<uint64_t>::max();

        vulkan->submit_work(submit_info, &done_rendering);

        return done_rendering.wait(timeout);
    }

    error_t gui_renderer_t::present_on_screen(uint64_t index)
    {
        return vulkan->present_on_screen(index, &render_finished);
    }


    error_t gui_renderer_t::init()
    {
        auto status = create_semaphores();

        if(status != error_t::success)
        {
            return status;
        }

        status = create_shaders();

        if(status != error_t::success)
        {
            return status;
        }

        status = create_renderpass();

        if(status != error_t::success)
        {
            return status;
        }

        status = create_descriptor_sets_layout();

        if(status != error_t::success)
        {
            return status;
        }

        status = create_descriptor_pool();

        if(status != error_t::success)
        {
            return status;
        }

        status = create_descriptor_sets();

        if(status != error_t::success)
        {
            return status;
        }

        status = create_pipeline_layout();

        if(status != error_t::success)
        {
            return status;
        }

        status = create_pipeline();

        if(status != error_t::success)
        {
            return status;
        }

        status = create_framebuffer();

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

        status = setup_font_texture();

        if(status != error_t::success)
        {
            return status;
        }

        status = allocate_buffers();

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

    error_t gui_renderer_t::render_frame()
    {
        gui->get_input();
        gui->run();

        auto status = create_buffers();

        if(status != error_t::success)
        {
            return status;
        }

        upload_gui_data();

        uint32_t fb_index;

        status = vulkan->get_next_swapchain_image(fb_index, &image_available);

        if(status != error_t::success)
        {
            return status;
        }

        status = build_command_buffer(fb_index);

        if(status != error_t::success)
        {
            return status;
        }

        status = submit_command_buffer(fb_index);

        if(status != error_t::success)
        {
            return status;
        }

        return present_on_screen(fb_index);
    }


    gui_renderer_t::~gui_renderer_t()
    {
        vulkan->destroy_pipeline(pipeline);
        vulkan->destroy_pipeline(render_output_pipeline);
        vulkan->destroy_pipeline_layout(pipeline_layout);
        vulkan->destroy_framebuffers(framebuffers);
        vulkan->destroy_render_pass(render_pass);
        vulkan->destroy_descriptor_set_layout(descriptor_set_layout);
        vulkan->destroy_descriptor_pool(descriptor_pool);
    }
}
