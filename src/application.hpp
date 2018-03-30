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


#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "window/window.hpp"
#include "vulkan/vulkan.hpp"

namespace bpmap
{

    class application_t
    {
        window_t window;
        vulkan_t vulkan;

        error_t error;

    public:

        application_t();

        void loop();
    };

}

#endif // APPLICATION_HPP
