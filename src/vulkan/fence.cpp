// Copyright 2023 Mihail Mladenov
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


#include "vulkan.hpp"
#include "fence.hpp"


namespace bpmap
{
    vk_fence_t::vk_fence_t()
    {
        dev = nullptr;
        fence = VK_NULL_HANDLE;
    }


    vk_fence_t::~vk_fence_t()
    {
        if(dev)
        {
            vkDestroyFence(dev->get_device(), fence, nullptr);
        }
    }

    error_t vk_fence_t::create(const vk_device_t& device)
    {
        VkFenceCreateInfo fci;
        fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fci.pNext = nullptr;
        fci.flags = 0;

        if(vkCreateFence(device.get_device(), &fci, nullptr, &fence) != VK_SUCCESS)
        {
            return error_t::fence_creation_fail;
        }

        dev = &device;

        return error_t::success;
    }

    error_t vk_fence_t::wait(uint64_t timeout)
    {
        auto status = vkWaitForFences(dev->get_device(), 1, &fence, VK_TRUE, timeout);

        if(status == VK_SUCCESS)
        {
            return error_t::success;
        }

        if(status == VK_TIMEOUT)
        {
            return error_t::timeout;
        }

        return error_t::device_lost;
    }
}
