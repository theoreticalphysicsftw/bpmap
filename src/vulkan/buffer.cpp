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


#include "buffer.hpp"


namespace bpmap
{
    error_t vk_buffer_t::map(void** data)
    {
        if(vmaMapMemory(allocator, allocation, data) != VK_SUCCESS)
        {
            return error_t::memory_mapping_fail;
        }

        return error_t::success;
    }


    void vk_buffer_t::unmap()
    {
        vmaUnmapMemory(allocator, allocation);
    }


    vk_buffer_t::~vk_buffer_t()
    {
        if(allocator != VK_NULL_HANDLE)
        {
            vmaDestroyBuffer(allocator, buffer, allocation);
        }
    }


    error_t vk_buffer_t::create(VmaAllocator alloc, const vk_buffer_desc_t& desc)
    {

        VkBufferCreateInfo bci = {};
        bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bci.pNext = nullptr;
        bci.flags = 0;
        bci.queueFamilyIndexCount = 0;
        bci.pQueueFamilyIndices = nullptr;
        bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bci.usage = desc.usage;
        bci.size = desc.size;

        VmaAllocationCreateInfo aci = {};
        aci.pool = VK_NULL_HANDLE;
        aci.flags = 0;
        aci.preferredFlags = 0;
        aci.requiredFlags = 0;
        aci.pUserData = nullptr;
        aci.usage = desc.on_gpu? VMA_MEMORY_USAGE_GPU_ONLY : VMA_MEMORY_USAGE_CPU_TO_GPU;

        if(
            vmaCreateBuffer(
                            alloc,
                            &bci,
                            &aci,
                            &buffer,
                            &allocation,
                            nullptr
                           )
           != VK_SUCCESS
          )
        {
            return error_t::buffer_creation_fail;
        }

        allocator = alloc;
        size = bci.size;

        return error_t::success;
    }
}
