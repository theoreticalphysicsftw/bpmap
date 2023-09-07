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


#include "fence.hpp"

namespace bpmap
{
    vk_fence_t::~vk_fence_t()
    {
        if(device != VK_NULL_HANDLE)
        {
            vkDestroyFence(device, fence, nullptr);
        }
    }

    error_t vk_fence_t::create(VkDevice device)
    {
        VkFenceCreateInfo fci;
        fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fci.pNext = nullptr;
        fci.flags = 0;

        if(vkCreateFence(device, &fci, nullptr, &fence) != VK_SUCCESS)
        {
            return error_t::fence_creation_fail;
        }

        this->device = device;

        return error_t::success;
    }
}
