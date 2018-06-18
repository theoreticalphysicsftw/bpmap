#include <core/io.hpp>

#include "renderer.hpp"

namespace bpmap
{

    void renderer_t::bind_vulkan(const vulkan_t& vk)
    {
        vulkan = &vk;
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

        status = create_compute_pipeline_layout();

        if(status != error_t::success)
        {
            return status;
        }

        status = create_compute_pipelines();

        if(status != error_t::success)
        {
            return status;
        }

        return error_t::success;
    }

    renderer_t::~renderer_t()
    {
        vulkan->destroy_shader(raytrace);
        vulkan->destroy_descriptor_pool(descriptor_pool);
        vulkan->destroy_descriptor_set_layout(descriptor_set_layout);
        vulkan->destroy_pipeline_layout(compute_pipeline_layout);
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

    error_t renderer_t::create_compute_pipeline_layout()
    {
        VkPipelineLayoutCreateInfo plci = {};
        plci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        plci.pNext = nullptr;
        plci.flags = 0;
        plci.pushConstantRangeCount = 0;
        plci.pPushConstantRanges = nullptr;
        plci.setLayoutCount = 1;
        plci.pSetLayouts = &descriptor_set_layout;

        return vulkan->create_pipeline_layout(compute_pipeline_layout, plci);
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
        cpci[raytrace_pipeline].layout = compute_pipeline_layout;
        cpci[raytrace_pipeline].basePipelineHandle = VK_NULL_HANDLE;
        cpci[raytrace_pipeline].basePipelineIndex = 0;

        return vulkan->create_compute_pipelines(compute_pipelines, cpci);
    }

}
