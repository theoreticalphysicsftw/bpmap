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

    const char_t** window_t::get_required_extensions(uint32_t& count)
    {
        return glfwGetRequiredInstanceExtensions(&count);
    }
}