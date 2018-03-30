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
           error = get_queue();
       }
       if(error == error_t::success)
       {
           error = create_command_pool();
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

    error_t vulkan_t::create_logical_device()
    {
        std::vector<VkPhysicalDevice> physical_devices;
        uint32_t physical_devices_count;

        vkEnumeratePhysicalDevices(instance, &physical_devices_count, nullptr);
        physical_devices.resize(physical_devices_count);
        vkEnumeratePhysicalDevices(instance, &physical_devices_count, physical_devices.data());

        // TODO: Be more reasonable in picking device
        auto appropriate_device = [this](const VkPhysicalDevice& pd)
        {
            VkPhysicalDeviceProperties dp;

            vkGetPhysicalDeviceProperties(pd,&dp);


            if(dp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                std::vector<VkQueueFamilyProperties> qfps;
                uint32_t count;

                vkGetPhysicalDeviceQueueFamilyProperties(pd, &count, nullptr);
                qfps.resize(count);
                vkGetPhysicalDeviceQueueFamilyProperties(pd, &count, qfps.data());

                for(auto idx = 0; idx < qfps.size(); ++idx)
                {
                    if(qfps[idx].queueFlags & VK_QUEUE_COMPUTE_BIT &&
                       qfps[idx].queueFlags & VK_QUEUE_GRAPHICS_BIT &&
                       qfps[idx].queueFlags & VK_QUEUE_TRANSFER_BIT &&
                       qfps[idx].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
                    {
                        queue_index = idx;
                        return true;
                    }
                }

                return false;
            }
            else
            {
                return false;
            }
        };

        VkPhysicalDevice pd;

        auto pdi = std::find_if(physical_devices.begin(), physical_devices.end(), appropriate_device);

        if(pdi != physical_devices.end())
        {
            pd = *pdi;
        }
        else
        {
            return error_t::device_search_fail;
        }


        VkDeviceQueueCreateInfo dqci = {};
        dqci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        dqci.flags = 0;
        dqci.pNext = nullptr;
        dqci.pQueuePriorities = nullptr;
        dqci.queueFamilyIndex = queue_index;
        dqci.queueCount = 1;

        VkDeviceCreateInfo dci = {};
        dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        dci.flags = 0;
        dci.pNext = nullptr;
        dci.enabledLayerCount = 0;
        dci.ppEnabledLayerNames = nullptr;
        dci.enabledExtensionCount = 0;
        dci.ppEnabledExtensionNames = nullptr;
        dci.queueCreateInfoCount = 1;
        dci.pQueueCreateInfos = &dqci;
        dci.pEnabledFeatures = nullptr;

        if (vkCreateDevice(pd, &dci, nullptr, &device) == VK_SUCCESS)
        {
            return error_t::success;
        }
        else
        {
            return error_t::logical_device_creation_fail;
        }
    }

    error_t vulkan_t::get_queue()
    {
        vkGetDeviceQueue(device, queue_index, 0, &queue);

        if(queue = VK_NULL_HANDLE)
        {
            return error_t::get_queue_fail;
        }

        return error_t::success;
    }

    error_t vulkan_t::create_command_pool()
    {
        VkCommandPoolCreateInfo cpci = {};
        cpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cpci.queueFamilyIndex = queue_index;
        cpci.flags = 0;
        cpci.pNext = nullptr;

        if(vkCreateCommandPool(device, &cpci, nullptr, &command_pool) != VK_SUCCESS)
        {
            return error_t::command_pool_creation_fail;
        }

        return error_t::success;
    }

    vulkan_t::~vulkan_t()
    {
        vkDeviceWaitIdle(device);
        vkDestroyCommandPool(device, command_pool, nullptr);
        vkDestroyDevice(device, nullptr);
        vkDestroyInstance(instance, nullptr);
    }
}
