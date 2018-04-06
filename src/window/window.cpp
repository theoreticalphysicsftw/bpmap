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


#include "window.hpp"
#include <cstdio>

namespace bpmap
{

    error_t window_t::init(window_init_params_t wip)
    {
        parameters = wip;

        if(!glfwInit())
        {
            return error_t::window_creation_fail;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(
                                   wip.width,
                                   wip.height,
                                   wip.name.c_str(),
                                   nullptr,
                                   nullptr
                                  );

        return error_t::success;
    }

    bool window_t::closed()
    {
        return glfwWindowShouldClose(window);
    }

    void window_t::poll_events()
    {
        glfwPollEvents();
    }

    darray_t<const char_t*> window_t::get_required_extensions() const
    {
        uint32_t count;
        auto extensions = glfwGetRequiredInstanceExtensions(&count);

        return darray_t<const char_t*>(extensions, extensions + count);
    }

    bool window_t::queue_supports_presentation(
                                               VkInstance instance,
                                               VkPhysicalDevice device,
                                               uint32_t queue_family
                                              ) const
    {
        return glfwGetPhysicalDevicePresentationSupport(instance, device, queue_family);
    }

    error_t window_t::create_surface(VkInstance instance, VkSurfaceKHR& surface)
    {
        if(glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
        {
            return error_t::surface_creation_fail;
        }

        return error_t::success;
    }

    bool_t window_t::mouse_button_pressed(mouse_button_t button) const
    {
        int_t glfw_button;

        switch(button)
        {
            case mouse_button_t::left: glfw_button = GLFW_MOUSE_BUTTON_LEFT; break;
            case mouse_button_t::right: glfw_button = GLFW_MOUSE_BUTTON_RIGHT; break;
            case mouse_button_t::middle: glfw_button = GLFW_MOUSE_BUTTON_MIDDLE; break;
        }

        return glfwGetMouseButton(window, glfw_button);
    }

    pair_t<int_t, int_t> window_t::get_mouse_pos() const
    {
        pair_t<int_t, int_t> result;

        double_t x;
        double_t y;

        glfwGetCursorPos(window, &x, &y);

        result.first = x;
        result.second = y;

        return result;
    }
}
