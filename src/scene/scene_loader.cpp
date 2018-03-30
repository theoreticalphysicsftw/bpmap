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

#include <cctype>
#include <io.hpp>
#include <algebra.hpp>

#define INI_IMPLEMENTATION

#include <ini.h>
#include <tiny_obj_loader.h>

#include "scene_loader.hpp"


namespace bpmap
{
    class scene_loader_t
    {
        ini_t* parsed;
        scene_t* scene;
        bool_t success;

    public:
        bool_t is_loaded() { return success; }

        scene_loader_t(const darray_t<uint8_t>& sd, scene_t& s)
        {
            scene = &s;
            success = false;

            parsed = ini_load((const char_t*) sd.data(), nullptr);

            if(!parsed)
            {
                return;
            }

            if(!load_settings())
            {
                return;
            }

            if(!load_camera())
            {
                return;
            }

            if(!load_lights())
            {
                return;
            }

            if(!load_objects())
            {
                return;
            }

            success = true;
        }

        ~scene_loader_t()
        {
            ini_destroy(parsed);
        }

    private:

        bool_t load_settings()
        {
            return true;
        }


        bool_t load_lights()
        {
            return true;
        }


        bool_t load_objects()
        {
            return true;
        }

        bool_t load_camera()
        {
            return true;
        }
    };


    bool_t load_scene(const string_t& path, scene_t& scene)
    {
        darray_t<uint8_t> scene_description;

        if(!read_whole_file(path, scene_description))
        {
            return false;
        }

        scene_description.push_back('\0');

        return scene_loader_t(scene_description, scene).is_loaded();
    }
}
