// Copyright 2023 Mihail Mladenov
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


#include "shader_registry.hpp"


namespace bpmap
{
    error_t shader_registry_t::add(
                                    const string_t& name,
                                    const uint32_t* code,
                                    size_t size,
                                    vk_shader_stage_t type
                                  )
    {
        if (name_to_id.find(name) != name_to_id.end())
        {
            return error_t::success;
        }

        vk_shader_id_t id = shaders.size();

        shaders.emplace_back();
        
        auto& shader = shaders.back();

        auto status = shader.create(*dev, code, size, type);

        if (status != error_t::success)
        {
            shaders.pop_back();
            return status;
        }

        name_to_id.emplace(name, id); 

        return error_t::success;
    }

    error_t shader_registry_t::add_from_file(const string_t& name, vk_shader_stage_t type)
    {
        darray_t<uint8_t> shader_data;

        if (!read_whole_file(name, shader_data))
        {
            return error_t::shader_read_fail;
        }

        add(name, (uint32_t*)shader_data.data(), shader_data.size(), type);

        return error_t::success;
    }


    error_t shader_registry_t::add_from_file(const desc_array_t& descs)
    {
        for (auto& pair : descs)
        {
            if (add_from_file(pair.first, pair.second) != error_t::success)
            {
                return error_t::shader_read_fail;
            }
        }

        return error_t::success;
    }


    vk_shader_t& shader_registry_t::get(const string_t& name)
    {
        DEBUG_VERIFY(name_to_id.find(name) != name_to_id.end());
        return shaders[name_to_id[name]];
    }
}
