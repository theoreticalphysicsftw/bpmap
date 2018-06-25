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

    application_t::application_t()
    {
        error = window.init({1280,720, app_name});

        if(error != error_t::success)
        {
            return;
        }

        // TODO: remove hardcoded scene and add an option to select from UI.
        error = load_scene("scene.bpmap", scene);

        if(error != error_t::success)
        {
            return;
        }

        if(error == error_t::success)
        {
            error = vulkan.init(window);
            gui.bind_window(window);

            renderer.bind_vulkan(vulkan);
            renderer.bind_scene(scene);

            if(error == error_t::success)
            {
                error = renderer.init();


                gui_renderer.bind_gui(gui);
                gui_renderer.bind_vulkan(vulkan);
                gui_renderer.bind_renderer(renderer);

                if(error == error_t::success)
                {
                     error = gui_renderer.init();
                }
            }

            renderer.build_command_buffers();
            renderer.submit_command_buffers();
        }
    }

    void application_t::loop()
    {
        if(error != error_t::success)
        {
            std::cerr<<"Error: "<<get_error_message(error)<<std::endl;
            std::exit(~0);
        }

        static constexpr uint32_t frame_limit = 60;

        auto t0 = std::chrono::high_resolution_clock::now();

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
        }
    }
}
