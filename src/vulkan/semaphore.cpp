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
#include "semaphore.hpp"


namespace bpmap::vk
{
    error_t semaphore_t::create(const device_t& device)
    {
        VkSemaphoreCreateInfo sci = {};
        sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        sci.pNext = nullptr;
        sci.flags = 0;

        if(vkCreateSemaphore(device.get_device(), &sci, nullptr, &semaphore) != VK_SUCCESS)
        {
            return error_t::semaphore_creation_fail;
        }

        dev = &device;

        return error_t::success;
    }


    semaphore_t::semaphore_t()
    {
        dev = nullptr;
        semaphore = VK_NULL_HANDLE;
    }


    semaphore_t::~semaphore_t()
    {
        if(dev)
        {
            vkDestroySemaphore(dev->get_device(), semaphore, nullptr);
        }
    }
}
