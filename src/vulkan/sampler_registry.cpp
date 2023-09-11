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


#include "sampler_registry.hpp"


namespace bpmap::vk
{
    error_t sampler_registry_t::add(const sampler_desc_t& desc)
    {
        if (desc_to_id.find(desc) != desc_to_id.end())
        {
            return error_t::success;
        }

        uint32_t id = samplers.size();

        samplers.emplace_back();
        
        auto& sampler = samplers.back();

        auto status = sampler.create(*dev, desc);

        if (status != error_t::success)
        {
            samplers.pop_back();
            return status;
        }

        desc_to_id.emplace(desc, id); 

        return error_t::success;
    }
    

    sampler_t& sampler_registry_t::get(const sampler_desc_t& desc)
    {
        DEBUG_VERIFY(desc_to_id.find(desc) != desc_to_id.end());
        return samplers[desc_to_id[desc]];
    }
}
