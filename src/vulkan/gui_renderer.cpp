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

        VkImageSubresourceRange isr = {};
        isr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        isr.baseArrayLayer = 0;
        isr.baseMipLevel = 0;
        isr.layerCount = 1;
        isr.levelCount = 1;

        VkImageViewCreateInfo ivci = {};
        ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ivci.pNext = nullptr;
        ivci.flags = 0;
        ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
        ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivci.subresourceRange = isr;

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


    void gui_renderer_t::bind_gui(const gui_t &g)
    {
        gui = &g;
    }

    void gui_renderer_t::bind_vulkan(const vulkan_t &vk)
    {
        vulkan = &vk;
    }

    error_t gui_renderer_t::init()
    {
        setup_font_texture();

        return error_t::success;
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
        viad[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;

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
        gpci.layout = layout;

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

    gui_renderer_t::~gui_renderer_t()
    {
        vulkan->destroy_image_view(font_view);
        vulkan->destroy_sampler(font_sampler);
    }
}
