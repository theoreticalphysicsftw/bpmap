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


#ifndef VULKAN_BUFFER_HPP
#define VULKAN_BUFFER_HPP


namespace bpmap
{

    struct vk_buffer_desc_t
    {
        size_t size = 0;
        uint32_t usage = 0;
        bool_t on_gpu = true; 
    };

    static constexpr uint32_t vk_buffer_usage_transfer_src =
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    static constexpr uint32_t vk_buffer_usage_transfer_dst =
        VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    static constexpr uint32_t vk_buffer_usage_uniform_buffer =
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    static constexpr uint32_t vk_buffer_usage_storage_buffer =
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    static constexpr uint32_t vk_buffer_usage_index_buffer =
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    static constexpr uint32_t vk_buffer_usage_vertex_buffer =
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    class vk_buffer_t
    {
        const vk_device_t* dev;
        VmaAllocation allocation;
        VkBuffer buffer;
        size_t size;

        vk_buffer_t(const vk_buffer_t&) = delete;
        vk_buffer_t& operator=(const vk_buffer_t&) = delete;

    public:
        VkBuffer get_handle() const { return buffer; }
        size_t get_size() const { return size; }
        error_t create(const vk_device_t& device, const vk_buffer_desc_t& desc);

        error_t map(void** data);
        void unmap();

        vk_buffer_t();
        ~vk_buffer_t();
    };



}

#endif
