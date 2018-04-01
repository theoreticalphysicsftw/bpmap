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


#include <algorithm>

#include "vulkan.hpp"

namespace bpmap
{

    error_t vulkan_t::init(window_t& win)
    {
        window = &win;

        error = create_instance();

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
           error = create_command_pools();
       }

       if(error == error_t::success)
       {
           error = create_surface_and_swapchain();
       }

       return error;
    }

    error_t vulkan_t::create_instance()
    {
        VkApplicationInfo appinfo = {};
        appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appinfo.apiVersion = VK_API_VERSION_1_0;
        appinfo.applicationVersion = 1;
        appinfo.pApplicationName = app_name;
        appinfo.engineVersion = 1;
        appinfo.pEngineName = app_name;
        appinfo.pNext = nullptr;

        VkInstanceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        info.pApplicationInfo = &appinfo;
        info.pNext = nullptr;
        info.enabledLayerCount = 0;
        info.ppEnabledLayerNames = nullptr;
        info.ppEnabledExtensionNames = window->get_required_extensions(info.enabledExtensionCount);
        info.flags = 0;

        if(vkCreateInstance(&info, nullptr, &instance) == VK_SUCCESS)
        {
            return error_t::success;
        }
        else
        {
            return error_t::instance_creation_fail;
        }
    }

    error_t vulkan_t::find_gpu(VkPhysicalDevice& gpu)
    {
        std::vector<VkPhysicalDevice> physical_devices;
        std::vector<std::pair<VkPhysicalDevice, uint32_t>> gpus;
        uint32_t physical_devices_count;

        vkEnumeratePhysicalDevices(instance, &physical_devices_count, nullptr);
        physical_devices.resize(physical_devices_count);
        vkEnumeratePhysicalDevices(instance, &physical_devices_count, physical_devices.data());

        for(auto physical_device : physical_devices)
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(physical_device, &properties);

            if(properties.vendorID == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                gpus.push_back(std::make_pair(physical_device,0));
            }

            if(properties.vendorID == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            {
                gpus.push_back(std::make_pair(physical_device,1));
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

    error_t vulkan_t::create_logical_device()
    {
        VkPhysicalDevice gpu;

        if(find_gpu(gpu) != error_t::success)
        {
            return error_t::device_search_fail;
        }

        auto select_graphics = [this, gpu](const VkQueueFamilyProperties& properties, size_t index)
        {
            return window->queue_supports_presentation(instance, gpu, index)
                   && properties.queueFlags == VK_QUEUE_GRAPHICS_BIT;
        };

        auto select_compute = [](const VkQueueFamilyProperties& properties, size_t index)
        {
            return properties.queueCount == VK_QUEUE_COMPUTE_BIT;
        };

        auto select_copy = [](const VkQueueFamilyProperties& properties, size_t index)
        {
            return properties.queueCount == VK_QUEUE_TRANSFER_BIT;
        };

        if(find_queue(gpu, graphics_queue_index, select_graphics) != error_t::success)
        {
            return error_t::device_search_fail;
        }

        if(find_queue(gpu, compute_queue_index, select_compute) != error_t::success)
        {
            return error_t::device_search_fail;
        }

        if(find_queue(gpu, copy_queue_index, select_copy) != error_t::success)
        {
            return error_t::device_search_fail;
        }

        VkDeviceQueueCreateInfo dqci[3] = {};

        dqci[graphics_queue].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        dqci[graphics_queue].flags = 0;
        dqci[graphics_queue].pNext = nullptr;
        dqci[graphics_queue].pQueuePriorities = nullptr;
        dqci[graphics_queue].queueFamilyIndex = graphics_queue_index;
        dqci[graphics_queue].queueCount = 1;

        dqci[compute_queue].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        dqci[compute_queue].flags = 0;
        dqci[compute_queue].pNext = nullptr;
        dqci[compute_queue].pQueuePriorities = nullptr;
        dqci[compute_queue].queueFamilyIndex = compute_queue_index;
        dqci[compute_queue].queueCount = 1;

        dqci[copy_queue].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        dqci[copy_queue].flags = 0;
        dqci[copy_queue].pNext = nullptr;
        dqci[copy_queue].pQueuePriorities = nullptr;
        dqci[copy_queue].queueFamilyIndex = copy_queue_index;
        dqci[copy_queue].queueCount = 1;


        VkDeviceCreateInfo dci = {};
        dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        dci.flags = 0;
        dci.pNext = nullptr;
        dci.enabledLayerCount = 0;
        dci.ppEnabledLayerNames = nullptr;
        dci.enabledExtensionCount = 0;
        dci.ppEnabledExtensionNames = nullptr;
        dci.queueCreateInfoCount = 3;
        dci.pQueueCreateInfos = dqci;
        dci.pEnabledFeatures = nullptr;

        if (vkCreateDevice(gpu, &dci, nullptr, &device) == VK_SUCCESS)
        {
            return error_t::success;
        }
        else
        {
            return error_t::logical_device_creation_fail;
        }
    }

    error_t vulkan_t::get_queues()
    {
        vkGetDeviceQueue(
                          device,
                          compute_queue_index,
                          compute_queue,
                          &queues[compute_queue]
                         );

        vkGetDeviceQueue(
                          device,
                          graphics_queue_index,
                          graphics_queue,
                          &queues[graphics_queue]
                        );

        vkGetDeviceQueue(
                          device,
                          copy_queue_index,
                          copy_queue,
                          &queues[copy_queue]
                        );

        if(std::any_of(
                        queues.begin(),
                        queues.end(),
                        [](const VkQueue& q)
                        {
                            return q == VK_NULL_HANDLE;
                        }
                        )
           )
        {
            return error_t::get_queue_fail;
        }

        return error_t::success;
    }

    error_t vulkan_t::create_command_pools()
    {
        VkCommandPoolCreateInfo compute_cpci = {};
        compute_cpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        compute_cpci.queueFamilyIndex = compute_queue_index;
        compute_cpci.flags = 0;
        compute_cpci.pNext = nullptr;

        VkCommandPoolCreateInfo graphics_cpci = {};
        graphics_cpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        graphics_cpci.queueFamilyIndex = graphics_queue_index;
        graphics_cpci.flags = 0;
        graphics_cpci.pNext = nullptr;

        VkCommandPoolCreateInfo copy_cpci = {};
        copy_cpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        copy_cpci.queueFamilyIndex = copy_queue_index;
        copy_cpci.flags = 0;
        copy_cpci.pNext = nullptr;

        if(
           vkCreateCommandPool(
                               device,
                               &compute_cpci,
                               nullptr,
                               &command_pools[compute_queue]
                              )
           != VK_SUCCESS
          )
        {
            return error_t::command_pool_creation_fail;
        }

        if(
           vkCreateCommandPool(
                               device,
                               &graphics_cpci,
                               nullptr,
                               &command_pools[graphics_queue]
                              )
           != VK_SUCCESS
          )
        {
            return error_t::command_pool_creation_fail;
        }

        if(
           vkCreateCommandPool(
                               device,
                               &copy_cpci,
                               nullptr,
                               &command_pools[copy_queue]
                              )
           != VK_SUCCESS
          )
        {
            return error_t::command_pool_creation_fail;
        }

        return error_t::success;
    }

    error_t vulkan_t::create_surface_and_swapchain()
    {
        auto status = window->create_surface(instance, surface);

        if(status != error_t::success)
        {
            return status;
        }

        VkExtent2D image_extent = {};
        image_extent.height = window->get_height();
        image_extent.width = window->get_width();

        //TODO: Validate presentation mode and image format.
        swapchain_image_format = VK_FORMAT_R8G8B8A8_UNORM;

        VkSwapchainCreateInfoKHR scci = {};
        scci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        scci.pNext = nullptr;
        scci.surface = surface;
        scci.minImageCount = 2;
        scci.flags = 0;
        scci.imageFormat = swapchain_image_format;
        scci.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        scci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        scci.imageExtent = image_extent;
        scci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        scci.imageArrayLayers = 1;
        scci.oldSwapchain = VK_NULL_HANDLE;
        scci.queueFamilyIndexCount = 0;
        scci.pQueueFamilyIndices = nullptr;
        scci.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
        scci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        scci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        scci.clipped = VK_TRUE;

        if(vkCreateSwapchainKHR(device, &scci, nullptr, &swapchain) != VK_SUCCESS)
        {
            return error_t::swapchain_creation_fail;
        }

        return get_swapchain_images();
    }

    error_t vulkan_t::get_swapchain_images()
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

    vulkan_t::~vulkan_t()
    {
        vkDeviceWaitIdle(device);

        std::for_each(
                      command_pools.begin(),
                      command_pools.end(),
                      [this](const VkCommandPool& command_pool)
                      {
                           vkDestroyCommandPool(device, command_pool, nullptr);
                      }
                     );

        std::for_each(
                      swapchain_image_views.begin(),
                      swapchain_image_views.end(),
                      [this](const VkImageView& image_view)
                      {
                           vkDestroyImageView(device, image_view, nullptr);
                      }
                     );

        vkDestroySwapchainKHR(device, swapchain, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyDevice(device, nullptr);
        vkDestroyInstance(instance, nullptr);
    }
}
