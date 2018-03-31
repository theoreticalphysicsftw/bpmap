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


#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <vulkan/vulkan.h>
#include <glfw3.h>
#include <common.hpp>
#include <error.hpp>

namespace bpmap
{

    struct window_init_params_t
    {
        uint32_t width;
        uint32_t height;
        string_t name;
    };

    class window_t
    {
        GLFWwindow* window;
        window_init_params_t parameters;
        error_t error;

    public:

        error_t init(window_init_params_t wip);
        bool_t closed();
        error_t get_status() const { return error; }
        void poll_events();

       const char_t** get_required_extensions(uint32_t&) const;
       bool queue_supports_presentation(VkInstance, VkPhysicalDevice, uint32_t) const;

       error_t create_surface(VkInstance instance, VkSurfaceKHR& surface);

       uint32_t get_height(){ return parameters.height; }
       uint32_t get_width(){ return parameters.width; }
    };

}
#endif // WINDOW_HPP
