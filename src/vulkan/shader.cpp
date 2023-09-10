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


#include "vulkan.hpp"
#include "shader.hpp"


namespace bpmap
{
    vk_shader_t::vk_shader_t() :
        type(vk_shader_stage_t::invalid),
        dev(nullptr),
        shader(VK_NULL_HANDLE)
    {
    }

    vk_shader_t::~vk_shader_t()
    {
        if (shader != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(dev->get_device(), shader, nullptr);
        }
    }


    error_t vk_shader_t::create(
                                  const vk_device_t& device,
                                  const uint32_t* data,
                                  size_t size,
                                  vk_shader_stage_t type
                                )
    {
        VkShaderModuleCreateInfo smci = {};
        smci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        smci.pNext = nullptr;
        smci.flags = 0;
        smci.codeSize = size;
        smci.pCode = data;

        if(vkCreateShaderModule(device.get_device(), &smci, nullptr, &shader) != VK_SUCCESS)
        {
            return error_t::shader_creation_fail;
        }

        dev = &device;

        return error_t::success;
    }
}
