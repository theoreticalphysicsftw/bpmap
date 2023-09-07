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

#include <cstdlib>
#include <iostream>
#include <chrono>

#include "application.hpp"

namespace bpmap
{

    application_t::application_t(
                                  uint32_t res_x,
                                  uint32_t res_y,
                                  const string_t& app_name
                                ) :
        gui(window),
        renderer(vulkan, scene, shader_registry),
        gui_renderer(gui, vulkan, shader_registry, renderer),
        shader_registry(vulkan)
    {
        verify(window.init({res_x, res_y, app_name}));

        // TODO: remove hardcoded scene and add an option to select from UI.
        verify(load_scene("scene.bpmap", scene));
        verify(vulkan.init(window));

        verify(renderer.init());

        verify(gui_renderer.init());
        verify(renderer.build_command_buffers());
        verify(renderer.submit_command_buffers());
    }

    void application_t::loop()
    {
        static constexpr uint32_t frame_limit = 60;

        auto t0 = std::chrono::high_resolution_clock::now();

        uint32_t frames = 0;
        while(!window.closed())
        {
            window.poll_events();
            auto t1 = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();

            if(duration >= 1E9 / frame_limit)
            {
                t0 = t1;
                gui_renderer.render_frame();
            }
            frames++;
        }
    }
}
