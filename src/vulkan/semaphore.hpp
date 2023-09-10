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


#ifndef VULKAN_SEMAPHORE_HPP
#define VULKAN_SEMAPHORE_HPP

namespace bpmap
{
    class vk_semaphore_t
    {
        const vk_device_t* dev;
        VkSemaphore semaphore;

        vk_semaphore_t(const vk_semaphore_t&) = delete;
        vk_semaphore_t& operator=(const vk_semaphore_t) = delete;

    public:
        VkSemaphore get_handle() const { return semaphore; }
        error_t create(const vk_device_t& device);

        vk_semaphore_t();
        ~vk_semaphore_t();
    };
}

#endif
