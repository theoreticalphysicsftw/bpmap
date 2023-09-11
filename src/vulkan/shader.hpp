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


#ifndef SHADER_HPP
#define SHADER_HPP


namespace bpmap::vk
{
    using shader_id_t = size_t;

    enum class shader_stage_t
    {
        vertex = 0,
        fragment,
        geometry,
        compute,
        invalid
    };

    class shader_t
    {
        const device_t* dev;
        shader_stage_t type;
        VkShaderModule shader;

        shader_t(const shader_t& other) = delete;
        shader_t& operator=(const shader_t& other) = delete;

        public:

        shader_t();
        ~shader_t();
        VkShaderModule get_handle() const { return shader; }
        shader_stage_t get_type() const { return type; }

        error_t create(
                        const device_t& device,
                        const uint32_t* data,
                        size_t size,
                        shader_stage_t type
                      );
    };
}

#endif
