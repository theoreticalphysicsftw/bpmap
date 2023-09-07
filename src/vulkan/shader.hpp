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


#include <common.hpp>
#include <error.hpp>

#include <vulkan/vulkan.h>


namespace bpmap
{
    enum class shader_stage_t
    {
        vertex = 0,
        fragment,
        geometry,
        compute,
        invalid
    };

    class vk_shader_t
    {
        shader_stage_t type = shader_stage_t::invalid;
        VkShaderModule shader = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;

        public:

        ~vk_shader_t();
        VkShaderModule get_handle() const { return shader; }
        error_t create(
                        VkDevice dev,
                        const uint32_t* data,
                        size_t size,
                        shader_stage_t type
                      );
    };
};

#endif
