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


#include "error.hpp"

namespace bpmap
{
    string_t get_error_message(error_t e)
    {
        switch(e)
        {
            case error_t::window_creation_fail :
                return "Failed to create window!";

            case error_t::instance_creation_fail :
                return "Failed to create vulkan instance!";

            case error_t::device_search_fail :
                return "Failed to find suitable physical device!";

            case error_t::logical_device_creation_fail:
                return "Failed to create logical device!";

            case error_t::get_queue_fail:
                return "Failed to get device queue!";

            case error_t::command_pool_creation_fail:
                return "Failed to create command pool!";
        }
    }
}
