// Copyright 2018-2023 Mihail Mladenov
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


#include <limits>

#define VMA_IMPLEMENTATION

#include "device.hpp"
#include "vulkan.hpp"

#include "vk.glslh"

namespace bpmap::vk
{
    error_t device_t::init(window_t& win, const device_desc_t& desc)
    {
        window = &win;

        error_t error;
        error = create_instance(win.get_name());

        if(error == error_t::success)
        {
            error = create_logical_device();
        }
        if(error == error_t::success)
        {
           error = get_queues();
        }
        if(error == error_t::success)
        {
           error = create_surface_and_swapchain();
        }
        if(error == error_t::success)
        {
           error = create_allocator();
        }
        if(error == error_t::success)
        {
           error = init_bindless_system(desc);
        }

       return error;
    }


    error_t device_t::create_pipeline_layout(
                                              VkPipelineLayout& layout,
                                              const VkPipelineLayoutCreateInfo& plci
                                            ) const
    {
        if(vkCreatePipelineLayout(device, &plci, nullptr,&layout) != VK_SUCCESS)
        {
            return error_t::pipeline_layout_creation_fail;
        }

        return error_t::success;
    }

    void device_t::destroy_pipeline_layout(VkPipelineLayout layout) const
    {
        vkDestroyPipelineLayout(device, layout, nullptr);
    }


    error_t device_t::create_graphics_pipeline(
                                                VkPipeline& pipeline,
                                                const VkGraphicsPipelineCreateInfo& gpci
                                              ) const
    {
        if(
            vkCreateGraphicsPipelines(
                                       device,
                                       VK_NULL_HANDLE,
                                       1,
                                       &gpci,
                                       nullptr,
                                       &pipeline
                                      )
            != VK_SUCCESS
          )
        {
            return error_t::pipeline_creation_fail;
        }

        return error_t::success;
    }

    error_t device_t::create_compute_pipelines(
                                               darray_t<VkPipeline>& pipelines,
                                               const darray_t<VkComputePipelineCreateInfo>& cpci
                                             ) const
    {
        if(
            vkCreateComputePipelines(
                                      device,
                                      VK_NULL_HANDLE,
                                      pipelines.size(),
                                      cpci.data(),
                                      nullptr,
                                      pipelines.data()
                                    )
             != VK_SUCCESS
           )
        {
            return error_t::pipeline_creation_fail;
        }

        return error_t::success;
    }

    void device_t::destroy_pipeline(VkPipeline pipeline) const
    {
        vkDestroyPipeline(device, pipeline, nullptr);
    }

    error_t device_t::create_renederpass(
                                          VkRenderPass &render_pass,
                                          const VkRenderPassCreateInfo &rpci
                                        ) const
    {
        if(vkCreateRenderPass(device, &rpci, nullptr, &render_pass) != VK_SUCCESS)
        {
            return error_t::render_pass_creation_fail;
        }

        return error_t::success;
    }

    void device_t::destroy_render_pass(VkRenderPass render_pass) const
    {
        vkDestroyRenderPass(device, render_pass, nullptr);
    }

    error_t device_t::create_framebuffers(
                                           darray_t<VkFramebuffer>& framebuffers,
                                           VkFramebufferCreateInfo &fbci
                                         ) const
    {
        fbci.height = window->get_height();
        fbci.width = window->get_width();
        fbci.layers = 1;
        fbci.attachmentCount = 1;

        framebuffers.resize(swapchain_image_views.size());

        for(auto i = 0u; i < swapchain_image_views.size(); ++i)
        {
            fbci.pAttachments = &swapchain_image_views[i];

            if(vkCreateFramebuffer(device, &fbci, nullptr, &framebuffers[i]) != VK_SUCCESS)
            {
                return error_t::framebuffer_creation_fail;
            }
        }

        return error_t::success;
    }

    void device_t::destroy_framebuffers(const darray_t<VkFramebuffer> &framebuffers) const
    {
        for(auto framebuffer: framebuffers)
        {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }
    }


    error_t device_t::create_instance(const string_t& name)
    {
        VkApplicationInfo appinfo = {};
        appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appinfo.apiVersion = VK_API_VERSION_1_2;
        appinfo.applicationVersion = 1;
        appinfo.pApplicationName = name.c_str();
        appinfo.engineVersion = 1;
        appinfo.pEngineName = name.c_str();
        appinfo.pNext = nullptr;

#if defined(BPMAP_DEBUG)
        uint32_t validation_layers_count = 1;
        const char_t* validation_layers[1] = {"VK_LAYER_KHRONOS_validation"};
#else
        uint32_t validation_layers_count = 0;
        const char_t* const* validation_layers = nullptr;
#endif
        auto extensions = window->get_required_extensions();

        VkInstanceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        info.pApplicationInfo = &appinfo;
        info.pNext = nullptr;
        info.enabledLayerCount = validation_layers_count;
        info.ppEnabledLayerNames = validation_layers;
        info.enabledExtensionCount = extensions.size();
        info.ppEnabledExtensionNames = extensions.data();
        info.flags = 0;

        auto result = vkCreateInstance(&info, nullptr, &instance);
        if(result == VK_SUCCESS)
        {
            return error_t::success;
        }
        else
        {
            return error_t::instance_creation_fail;
        }
    }


    error_t device_t::find_gpu(VkPhysicalDevice& gpu)
    {
        std::vector<VkPhysicalDevice> physical_devices;
        std::vector<std::pair<VkPhysicalDevice, uint32_t>> gpus;
        uint32_t physical_devices_count;

        auto result = vkEnumeratePhysicalDevices(instance, &physical_devices_count, nullptr);

        if (result != VK_SUCCESS)
        {
            return error_t::device_search_fail;
        }

        physical_devices.resize(physical_devices_count);
        vkEnumeratePhysicalDevices(instance, &physical_devices_count, physical_devices.data());

        for(auto physical_device : physical_devices)
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(physical_device, &properties);

            if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                gpus.push_back(std::make_pair(physical_device, 0));
            }

            if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            {
                gpus.push_back(std::make_pair(physical_device, 1));
            }
        }

        using pgpu_t = const std::pair<VkPhysicalDevice, uint32_t>&;
        auto comparator = [](pgpu_t x, pgpu_t y){return x.second < y.second;};

        std::sort(gpus.begin(), gpus.end(), comparator);

        if(!gpus.empty())
        {
            gpu = gpus.front().first;
        }
        else
        {
            return error_t::device_search_fail;
        }

        return error_t::success;
    }


    error_t device_t::create_logical_device()
    {
        if(find_gpu(gpu_device) != error_t::success)
        {
            return error_t::device_search_fail;
        }

        auto select_queue = [this](const VkQueueFamilyProperties& properties, size_t index)
        {
            return window->queue_supports_presentation(instance, gpu_device, index)
                   && properties.queueFlags & VK_QUEUE_GRAPHICS_BIT
                   && properties.queueFlags & VK_QUEUE_COMPUTE_BIT
                   && properties.queueFlags & VK_QUEUE_TRANSFER_BIT;
        };


        if(find_queue(gpu_device, queue_index, select_queue) != error_t::success)
        {
            return error_t::queue_search_fail;
        }

        float_t priority = 1.0;

        VkDeviceQueueCreateInfo dqci;
        dqci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        dqci.flags = 0;
        dqci.pNext = nullptr;
        dqci.pQueuePriorities = &priority;
        dqci.queueFamilyIndex = queue_index;
        dqci.queueCount = 1;

        std::vector<const char*> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        VkPhysicalDeviceFeatures features = {};
        features.samplerAnisotropy = VK_TRUE;
        features.shaderSampledImageArrayDynamicIndexing = VK_TRUE;
        features.shaderStorageBufferArrayDynamicIndexing = VK_TRUE;
        features.shaderStorageImageArrayDynamicIndexing = VK_TRUE;
        features.shaderStorageImageWriteWithoutFormat = VK_TRUE;
        features.shaderStorageImageReadWithoutFormat = VK_TRUE;

        VkPhysicalDeviceBufferDeviceAddressFeatures address_features = {};
        address_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
        address_features.pNext = nullptr;
        address_features.bufferDeviceAddress = VK_TRUE;

        VkPhysicalDeviceDescriptorIndexingFeatures indexing_features = {};
        indexing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
        indexing_features.pNext = &address_features;
        indexing_features.runtimeDescriptorArray = VK_TRUE;
        indexing_features.descriptorBindingPartiallyBound = VK_TRUE;
        indexing_features.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
        indexing_features.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
        indexing_features.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;

        VkDeviceCreateInfo dci = {};
        dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        dci.pNext = &indexing_features;
        dci.flags = 0;
        dci.enabledLayerCount = 0;
        dci.ppEnabledLayerNames = nullptr;
        dci.enabledExtensionCount = extensions.size();
        dci.ppEnabledExtensionNames = extensions.data();
        dci.queueCreateInfoCount = 1;
        dci.pQueueCreateInfos = &dqci;
        dci.pEnabledFeatures = &features;

        if (vkCreateDevice(gpu_device, &dci, nullptr, &device) == VK_SUCCESS)
        {
            return error_t::success;
        }
        else
        {
            return error_t::logical_device_creation_fail;
        }
    }


    error_t device_t::get_queues()
    {
        vkGetDeviceQueue(
                          device,
                          queue_index,
                          0,
                          &queue
                         );

        if(queue == VK_NULL_HANDLE)
        {
            return error_t::get_queue_fail;
        }

        return error_t::success;
    }


    error_t device_t::create_command_pool(command_pool_t& command_pool) const
    {
        VkCommandPoolCreateInfo cpci = {};
        cpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cpci.queueFamilyIndex = queue_index;
        cpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        cpci.pNext = nullptr;

        VkCommandPool pool;

        if(
            vkCreateCommandPool(
                                 device,
                                 &cpci,
                                 nullptr,
                                 &pool
                                )
            != VK_SUCCESS
          )
        {
            return error_t::command_pool_creation_fail;
        }

        command_pool.pool = pool;
        command_pool.device = device;

        return error_t::success;
    }

    error_t device_t::create_command_buffers(
                                              VkCommandBuffer* buffers,
                                              const VkCommandBufferAllocateInfo &cbai
                                            ) const
    {
        if(vkAllocateCommandBuffers(device, &cbai, buffers) != VK_SUCCESS)
        {
            return error_t::command_buffers_creation_fail;
        }

        return error_t::success;
    }


    error_t device_t::submit_work(const VkSubmitInfo& submit_info, const fence_t* fence) const
    {
        auto fence_handle = fence? fence->get_handle() : VK_NULL_HANDLE;
        if(vkQueueSubmit(queue, 1, &submit_info, fence_handle) != VK_SUCCESS)
        {
            return error_t::queue_submit_fail;
        }

        return error_t::success;
    }

    error_t device_t::wait_idle() const
    {
        if(vkQueueWaitIdle(queue) != VK_SUCCESS)
        {
            return error_t::queue_wait_fail;
        }

        return error_t::success;
    }

    error_t device_t::get_next_swapchain_image(uint32_t &fb_index, const semaphore_t* semaphore) const
    {
        vkAcquireNextImageKHR(
                               device,
                               swapchain,
                               std::numeric_limits<uint64_t>::max(),
                               semaphore->get_handle(),
                               VK_NULL_HANDLE,
                               &fb_index
                             );

        return error_t::success;
    }

    error_t device_t::present_on_screen(uint32_t index, const semaphore_t* wait_semaphore) const
    {
        auto wait_handle = wait_semaphore->get_handle();
        VkPresentInfoKHR present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.pNext = nullptr;
        present_info.pImageIndices = &index;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swapchain;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &wait_handle;
        present_info.pResults = nullptr;

        if(vkQueuePresentKHR(queue, &present_info) != VK_SUCCESS)
        {
            return error_t::presentation_fail;
        }

        return error_t::success;
    }

    error_t device_t::validate_surface_support()
    {
        VkBool32 supported;

        if(vkGetPhysicalDeviceSurfaceSupportKHR(gpu_device, queue_index, surface, &supported) != VK_SUCCESS)
        {
            return error_t::surface_validation_fail;
        }

        if(!supported)
        {
            return error_t::surface_validation_fail;
        }

        return error_t::success;
    }

    error_t device_t::pick_surface_format()
    {
        std::vector<VkSurfaceFormatKHR> formats;
        uint32_t count;

        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu_device, surface, &count, nullptr);
        formats.resize(count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu_device, surface, &count, formats.data());

        if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
        {
            swapchain_image_format = VK_FORMAT_B8G8R8A8_UNORM;
            swapchain_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

            return error_t::success;
        }

        for(auto format: formats)
        {
            if(
                format.format == VK_FORMAT_B8G8R8A8_UNORM &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
              )
            {
                swapchain_image_format = VK_FORMAT_B8G8R8A8_UNORM;
                swapchain_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

                return error_t::success;
            }
        }

        swapchain_image_format = formats[0].format;
        swapchain_color_space = formats[0].colorSpace;

        return error_t::success;
    }

    error_t device_t::pick_present_mode()
    {
        std::vector<VkPresentModeKHR> modes;
        uint32_t count;

        vkGetPhysicalDeviceSurfacePresentModesKHR(gpu_device, surface, &count, nullptr);
        modes.resize(count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(gpu_device, surface, &count, modes.data());

        for(auto present_mode: modes)
        {
            if(present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                swapchain_present_mode = present_mode;

                return error_t::success;
            }
        }

        swapchain_present_mode = VK_PRESENT_MODE_FIFO_KHR;

        return error_t::success;
    }


    error_t device_t::create_surface_and_swapchain()
    {
        auto status = window->create_surface(instance, surface);

        if(status != error_t::success)
        {
            return status;
        }

        status = validate_surface_support();

        if(status != error_t::success)
        {
            return status;
        }

        VkExtent2D image_extent = {};
        image_extent.height = window->get_height();
        image_extent.width = window->get_width();

        status = pick_surface_format();

        if(status != error_t::success)
        {
            return status;
        }

        status = pick_present_mode();

        if(status != error_t::success)
        {
            return status;
        }

        VkSwapchainCreateInfoKHR scci = {};
        scci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        scci.pNext = nullptr;
        scci.surface = surface;
        scci.minImageCount = 2;
        scci.flags = 0;
        scci.imageFormat = swapchain_image_format;
        scci.imageColorSpace = swapchain_color_space;
        scci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        scci.imageExtent = image_extent;
        scci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        scci.imageArrayLayers = 1;
        scci.oldSwapchain = VK_NULL_HANDLE;
        scci.queueFamilyIndexCount = 0;
        scci.pQueueFamilyIndices = nullptr;
        scci.presentMode = swapchain_present_mode;
        scci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        scci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        scci.clipped = VK_TRUE;

        if(vkCreateSwapchainKHR(device, &scci, nullptr, &swapchain) != VK_SUCCESS)
        {
            return error_t::swapchain_creation_fail;
        }

        return get_swapchain_images();
    }


    error_t device_t::get_swapchain_images()
    {

        uint32_t image_count;

        vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr);
        swapchain_images.resize(image_count);
        vkGetSwapchainImagesKHR(device, swapchain, &image_count, swapchain_images.data());

        swapchain_image_views.resize(image_count);

        for(auto i = 0; i < image_count; ++i)
        {
            VkImageSubresourceRange isr = {};
            isr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            isr.baseArrayLayer = 0;
            isr.baseMipLevel = 0;
            isr.layerCount = 1;
            isr.levelCount = 1;

            VkComponentMapping cm = {};
            cm.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            cm.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            cm.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            cm.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            VkImageViewCreateInfo ivci = {};
            ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            ivci.pNext = nullptr;
            ivci.flags = 0;
            ivci.format = swapchain_image_format;
            ivci.image = swapchain_images[i];
            ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
            ivci.subresourceRange = isr;
            ivci.components = cm;

            if(vkCreateImageView(device, &ivci, nullptr, &swapchain_image_views[i]) != VK_SUCCESS)
            {
                return error_t::swapchain_creation_fail;
            }
        }

        return error_t::success;
    }


    error_t device_t::create_allocator()
    {
        VmaAllocatorCreateInfo aci = {};
        aci.instance = instance;
        aci.device = device;
        aci.physicalDevice = gpu_device;
        aci.flags = 0;
        aci.pAllocationCallbacks = nullptr;
        aci.pDeviceMemoryCallbacks = nullptr;
        aci.pVulkanFunctions = nullptr;
        aci.pHeapSizeLimit = nullptr;
        aci.preferredLargeHeapBlockSize = 0;

        if(vmaCreateAllocator(&aci, &allocator) != VK_SUCCESS)
        {
            return error_t::allocator_creation_fail;
        }

        return error_t::success;
    }


    device_t::~device_t()
    {
        vkDeviceWaitIdle(device);

        destroy_bindless_system();

        vkDestroySwapchainKHR(device, swapchain, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vmaDestroyAllocator(allocator);
        vkDestroyDevice(device, nullptr);
        vkDestroyInstance(instance, nullptr);
    }


    command_pool_t::command_pool_t()
    {
        device = VK_NULL_HANDLE;
        pool = VK_NULL_HANDLE;
    }


    command_pool_t::~command_pool_t()
    {
        if(device != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(device, pool, nullptr);
        }
    }

    error_t device_t::init_bindless_system(const device_desc_t& desc)
    {
        push_range.stageFlags = VK_SHADER_STAGE_ALL;
        push_range.offset = 0;
        push_range.size = desc.max_push_constants * 4;

        VkDescriptorPoolSize pool_sizes[5] = {};

        pool_sizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        pool_sizes[0].descriptorCount = desc.max_bindless_images;
        pool_sizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        pool_sizes[1].descriptorCount = desc.max_bindless_images;
        pool_sizes[2].type = VK_DESCRIPTOR_TYPE_SAMPLER;
        pool_sizes[2].descriptorCount = desc.max_bindless_samplers;
        pool_sizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        pool_sizes[3].descriptorCount = desc.max_bindless_buffers;
        pool_sizes[4].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        pool_sizes[4].descriptorCount = 1;

        VkDescriptorPoolCreateInfo dpci = {};
        dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        dpci.pNext = nullptr;
        dpci.maxSets = 1;
        dpci.flags =
            VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT |
            VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
        dpci.poolSizeCount = 5;
        dpci.pPoolSizes = pool_sizes;

        if(vkCreateDescriptorPool(device, &dpci, nullptr, &bindless_pool) != VK_SUCCESS)
        {
            return error_t::descriptor_pool_creation_fail;
        }

        VkDescriptorSetLayoutBinding dslb[5] = {};

        dslb[0].binding = BINDLESS_SAMPLED_IMAGES_SLOT;
        dslb[0].descriptorCount = desc.max_bindless_images;
        dslb[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        dslb[0].stageFlags = VK_SHADER_STAGE_ALL;

        dslb[1].binding = BINDLESS_STORAGE_IMAGES_SLOT;
        dslb[1].descriptorCount = desc.max_bindless_images;
        dslb[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        dslb[1].stageFlags = VK_SHADER_STAGE_ALL;

        dslb[2].binding = BINDLESS_SAMPLERS_SLOT;
        dslb[2].descriptorCount = desc.max_bindless_samplers;
        dslb[2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        dslb[2].stageFlags = VK_SHADER_STAGE_ALL;

        dslb[3].binding = BINDLESS_BUFFERS_SLOT;
        dslb[3].descriptorCount = desc.max_bindless_buffers;
        dslb[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        dslb[3].stageFlags = VK_SHADER_STAGE_ALL;

        dslb[4].binding = BINDLESS_HANDLE_BUFFER_SLOT;
        dslb[4].descriptorCount = 1;
        dslb[4].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        dslb[4].stageFlags = VK_SHADER_STAGE_ALL;

        VkDescriptorBindingFlags flags[] =
        {
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
            0
        };

        VkDescriptorSetLayoutBindingFlagsCreateInfo dslbfci = {};
        dslbfci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        dslbfci.pNext = nullptr;
        dslbfci.pBindingFlags = flags;
        dslbfci.bindingCount = 5;

        VkDescriptorSetLayoutCreateInfo dslci = {};
        dslci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        dslci.pNext = &dslbfci;
        dslci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        dslci.bindingCount = 5;
        dslci.pBindings = dslb;

        if(vkCreateDescriptorSetLayout(device, &dslci, nullptr, &bindless_layout) != VK_SUCCESS)
        {
            return error_t::descriptor_set_layout_creation_fail;
        }

        VkDescriptorSetAllocateInfo dsai = {};
        dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        dsai.pNext = nullptr;
        dsai.descriptorSetCount = 1;
        dsai.pSetLayouts = &bindless_layout;
        dsai.descriptorPool = bindless_pool;

        if(vkAllocateDescriptorSets(device, &dsai, &bindless_set) != VK_SUCCESS)
        {
            return error_t::descriptor_set_allocation_fail;
        }

        return error_t::success;
    }


    uint32_t device_t::bind(const image_t* image) const
    {
        auto index = image_slots.allocate();

        VkDescriptorImageInfo sampled_info = {};
        sampled_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        sampled_info.imageView = image->get_view();
        sampled_info.sampler = VK_NULL_HANDLE;

        VkDescriptorImageInfo storage_info = sampled_info;
        storage_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.pNext = nullptr;
        write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        write.descriptorCount = 1;
        write.dstSet = bindless_set;
        write.dstBinding = BINDLESS_SAMPLED_IMAGES_SLOT;
        write.dstArrayElement = index;
        write.pBufferInfo = nullptr;
        write.pImageInfo = &sampled_info;
        write.pTexelBufferView = nullptr;

        uint32_t writes_count = 0;
        array_t<VkWriteDescriptorSet, 2> writes;

        if (image->get_info().usage | usage_sampled)
        {
            writes[writes_count++] = write; 
        }

        write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        write.dstBinding = BINDLESS_STORAGE_IMAGES_SLOT;
        write.pImageInfo = &storage_info;

        if (image->get_info().usage | usage_storage)
        {
            writes[writes_count++] = write; 
        }
        
        vkUpdateDescriptorSets(device, writes_count, writes.data(), 0, nullptr);
        return index;
    }


    uint32_t device_t::bind(const buffer_t* buffer) const
    {
        auto index = buffer_slots.allocate();

        VkDescriptorBufferInfo info = {};
        info.buffer = buffer->get_handle();
        info.offset = 0;
        info.range = VK_WHOLE_SIZE;

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.pNext = nullptr;
        write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        write.descriptorCount = 1;
        write.dstSet = bindless_set;
        write.dstBinding = BINDLESS_BUFFERS_SLOT;
        write.dstArrayElement = index;
        write.pBufferInfo = &info;
        write.pImageInfo = nullptr;
        write.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
        return index;
    }


    uint32_t device_t::bind(const sampler_t* sampler) const
    {
        auto index = sampler_slots.allocate();

        VkDescriptorImageInfo info = {};
        info.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        info.imageView = VK_NULL_HANDLE;
        info.sampler = sampler->get_handle();

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.pNext = nullptr;
        write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        write.descriptorCount = 1;
        write.dstSet = bindless_set;
        write.dstBinding = BINDLESS_SAMPLERS_SLOT;
        write.dstArrayElement = index;
        write.pBufferInfo = nullptr;
        write.pImageInfo = &info;
        write.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
        return index;
    }


    void device_t::destroy_bindless_system()
    {
        vkDestroyDescriptorPool(device, bindless_pool, nullptr);
        vkDestroyDescriptorSetLayout(device, bindless_layout, nullptr);
    }

}
