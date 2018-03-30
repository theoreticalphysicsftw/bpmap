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

#include "application.hpp"

namespace bpmap
{

    application_t::application_t()
    {
        error = window.init({1024,720, app_name});

        if(error != error_t::success)
        {
            error = vulkan.init(window);
        }
    }

    void application_t::loop()
    {
        if(error != error_t::success)
        {
            std::cerr<<"Error: "<<get_error_message(error)<<std::endl;
            std::exit(~0);
        }

        while(!window.closed())
        {
            window.poll_events();
        }
    }
}
