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


#ifndef VULKAN_HPP
#define VULKAN_HPP

#include <common.hpp>
#include <error.hpp>
#include <vulkan/vulkan.h>

#include "../window/window.hpp"

namespace bpmap
{
    class vulkan_t
    {
        VkInstance instance;
        VkDevice device;
        VkQueue queue;
        VkCommandPool command_pool;

        uint32_t queue_index;

        window_t* window;

        error_t error;

        error_t create_instance();
        error_t create_logical_device();
        error_t get_queue();
        error_t create_command_pool();

    public:

        error_t get_status() const { return error;}

        error_t init(window_t&);

        ~vulkan_t();
    };
}
#endif // VULKAN_HPP
