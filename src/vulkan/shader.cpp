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


#include "shader.hpp"


namespace bpmap
{
    vk_shader_t::~vk_shader_t()
    {
        if (shader != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(device, shader, nullptr);
        }
    }


    error_t vk_shader_t::create(
                                  VkDevice dev,
                                  const uint32_t* data,
                                  size_t size,
                                  shader_stage_t type
                                )
    {
        VkShaderModuleCreateInfo smci = {};
        smci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        smci.pNext = nullptr;
        smci.flags = 0;
        smci.codeSize = size;
        smci.pCode = data;

        if(vkCreateShaderModule(dev, &smci, nullptr, &shader) != VK_SUCCESS)
        {
            return error_t::shader_creation_fail;
        }

        device = dev;

        return error_t::success;
    }
}
