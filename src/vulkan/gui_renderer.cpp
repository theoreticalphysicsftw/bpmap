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

#include "gui_renderer.hpp"

namespace bpmap
{
    error_t gui_renderer_t::setup_font_texture()
    {
        VkExtent3D extent = {};
        extent.height = gui->get_font_height();
        extent.width = gui->get_font_width();
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
        ici.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
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

        if(vulkan->create_image(font_image, ici, aci) != error_t::success)
        {
            return error_t::font_texture_setup_fail;
        }

        vk_buffer_t staging_buffer;

        VkBufferCreateInfo staging_buffer_bci = {};
        staging_buffer_bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        staging_buffer_bci.pNext = nullptr;
        staging_buffer_bci.flags = 0;
        staging_buffer_bci.queueFamilyIndexCount = 0;
        staging_buffer_bci.pQueueFamilyIndices = nullptr;
        staging_buffer_bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        staging_buffer_bci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        staging_buffer_bci.size = gui->get_font_width() * gui->get_font_height() * 4;

        VmaAllocationCreateInfo staging_buffer_aci = {};
        staging_buffer_aci.pool = VK_NULL_HANDLE;
        staging_buffer_aci.flags = 0;
        staging_buffer_aci.preferredFlags = 0;
        staging_buffer_aci.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        staging_buffer_aci.pUserData = nullptr;
        staging_buffer_aci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        if(vulkan->create_buffer(staging_buffer, staging_buffer_bci, staging_buffer_aci) != error_t::success)
        {
            return error_t::font_texture_setup_fail;
        }

        void* mapped;
        auto status = staging_buffer.map(&mapped);

        if(status != error_t::success)
        {
            return status;
        }

        memcpy(mapped, gui->get_raw_font(), staging_buffer_bci.size);

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
        layout_barrier.image = font_image.image;
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
                                staging_buffer.buffer,
                                font_image.image,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                1,
                                &bic
                              );

        VkImageMemoryBarrier final_layout_barrier = {};
        final_layout_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        final_layout_barrier.pNext = nullptr;
        final_layout_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        final_layout_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        final_layout_barrier.image = font_image.image;
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

        if(vulkan->submit_work(submit_info) != error_t::success)
        {
            return error_t::font_texture_setup_fail;
        }

        vulkan->wait_idle();

        gui->finalize_font_atlas();

        VkImageViewCreateInfo ivci = {};
        ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ivci.pNext = nullptr;
        ivci.flags = 0;
        ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
        ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivci.subresourceRange = isr;
        ivci.image = font_image.image;

        if(vulkan->create_image_view(font_view,ivci) != error_t::success)
        {
            return error_t::font_texture_setup_fail;
        }


        VkSamplerCreateInfo sci = {};
        sci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sci.pNext = nullptr;
        sci.flags = 0;
        sci.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sci.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sci.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sci.anisotropyEnable = VK_FALSE;
        sci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sci.mipLodBias = 0.0;
        sci.compareEnable = VK_FALSE;
        sci.minFilter = VK_FILTER_LINEAR;
        sci.magFilter = VK_FILTER_LINEAR;
        sci.minLod = 0.0;
        sci.maxLod = 0.0;
        sci.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

        return vulkan->create_sampler(font_sampler, sci);
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
        pssci[0].module = vertex_shader;
        pssci[0].pSpecializationInfo = nullptr;

        pssci[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pssci[1].pNext = nullptr;
        pssci[1].pName = "main";
        pssci[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        pssci[1].module = fragment_shader;
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
        prsci.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        prsci.polygonMode = VK_POLYGON_MODE_FILL;

        VkPipelineMultisampleStateCreateInfo pmsci = {};
        pmsci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        pmsci.pNext = nullptr;
        pmsci.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;

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
        gpci.subpass = 0;
        gpci.layout = pipeline_layout;

        return vulkan->create_graphics_pipeline(pipeline, gpci);
    }


    error_t gui_renderer_t::create_renderpass()
    {
        VkAttachmentDescription attachment_description = {};
        attachment_description.flags = 0;
        attachment_description.format = vulkan->get_swapchain_format();
        attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference attachment_reference = {};
        attachment_reference.attachment = 0;
        attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.flags = 0;
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.inputAttachmentCount = 0;
        subpass.pInputAttachments = nullptr;
        subpass.preserveAttachmentCount = 0;
        subpass.pPreserveAttachments = nullptr;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &attachment_reference;
        subpass.pDepthStencilAttachment = nullptr;
        subpass.pResolveAttachments = nullptr;

        VkRenderPassCreateInfo rpci = {};
        rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        rpci.pNext = nullptr;
        rpci.flags = 0;
        rpci.attachmentCount = 1;
        rpci.pAttachments = &attachment_description;
        rpci.subpassCount = 1;
        rpci.pSubpasses = &subpass;
        rpci.dependencyCount = 0;
        rpci.pDependencies = nullptr;

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
        std::vector<uint8_t> vertex_shader_data;
        std::vector<uint8_t> fragment_shader_data;

        if(!read_whole_file(vertex_shader_path, vertex_shader_data))
        {
            return error_t::vertex_shader_read_fail;
        }

        if(!read_whole_file(fragment_shader_path, fragment_shader_data))
        {
            return error_t::fragment_shader_read_fail;
        }

        VkShaderModuleCreateInfo vertex_smci = {};
        vertex_smci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        vertex_smci.pNext = nullptr;
        vertex_smci.flags = 0;
        vertex_smci.codeSize = vertex_shader_data.size();
        vertex_smci.pCode = (uint32_t*) vertex_shader_data.data();

        VkShaderModuleCreateInfo fragment_smci = {};
        fragment_smci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        fragment_smci.pNext = nullptr;
        fragment_smci.flags = 0;
        fragment_smci.codeSize = fragment_shader_data.size();
        fragment_smci.pCode = (uint32_t*) fragment_shader_data.data();

        auto status = vulkan->create_shader(vertex_shader, vertex_smci);

        if(status != error_t::success)
        {
            return status;
        }

        return vulkan->create_shader(fragment_shader, fragment_smci);
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

    error_t gui_renderer_t::allocate_buffers()
    {
        VkBufferCreateInfo index_buffer_bci = {};
        index_buffer_bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        index_buffer_bci.pNext = nullptr;
        index_buffer_bci.flags = 0;
        index_buffer_bci.queueFamilyIndexCount = 0;
        index_buffer_bci.pQueueFamilyIndices = nullptr;
        index_buffer_bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        index_buffer_bci.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        index_buffer_bci.size = max_gui_ibuffer_size;

        VmaAllocationCreateInfo index_buffer_aci = {};
        index_buffer_aci.pool = VK_NULL_HANDLE;
        index_buffer_aci.flags = 0;
        index_buffer_aci.preferredFlags = 0;
        index_buffer_aci.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        index_buffer_aci.pUserData = nullptr;
        index_buffer_aci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        auto status = vulkan->create_buffer(index_buffer, index_buffer_bci, index_buffer_aci);

        if(status != error_t::success)
        {
            return status;
        }

        VkBufferCreateInfo vertex_buffer_bci = {};
        vertex_buffer_bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vertex_buffer_bci.pNext = nullptr;
        vertex_buffer_bci.flags = 0;
        vertex_buffer_bci.queueFamilyIndexCount = 0;
        vertex_buffer_bci.pQueueFamilyIndices = nullptr;
        vertex_buffer_bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vertex_buffer_bci.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        vertex_buffer_bci.size = max_gui_ibuffer_size;

        VmaAllocationCreateInfo vertex_buffer_aci = {};
        vertex_buffer_aci.pool = VK_NULL_HANDLE;
        vertex_buffer_aci.flags = 0;
        vertex_buffer_aci.preferredFlags = 0;
        vertex_buffer_aci.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        vertex_buffer_aci.pUserData = nullptr;
        vertex_buffer_aci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        status = vulkan->create_buffer(vertex_buffer, vertex_buffer_bci, vertex_buffer_aci);

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


    void gui_renderer_t::bind_gui(gui_t &g)
    {
        gui = &g;
    }


    void gui_renderer_t::bind_vulkan(const vulkan_t &vk)
    {
        vulkan = &vk;
    }


    error_t gui_renderer_t::init()
    {
        auto status = create_shaders();

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

        return error_t::success;
    }


    gui_renderer_t::~gui_renderer_t()
    {
        vulkan->destroy_image_view(font_view);
        vulkan->destroy_sampler(font_sampler);
    }
}
