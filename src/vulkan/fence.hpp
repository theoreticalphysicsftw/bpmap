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


#ifndef VULKAN_FENCE_HPP
#define VULKAN_FENCE_HPP


namespace bpmap::vk
{
    class fence_t
    {
        const device_t* dev;
        VkFence fence = VK_NULL_HANDLE;

        fence_t(const fence_t&) = delete;
        fence_t& operator=(const fence_t&) = delete;

    public:
        VkFence get_handle() const { return fence; }
        error_t create(const device_t& device);
        error_t wait(uint64_t timeout = 0);
        fence_t();
        ~fence_t();
    };
}

#endif
