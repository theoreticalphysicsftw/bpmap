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


#include <cstdio>

#include "io.hpp"

namespace bpmap
{
    using file_t = FILE*;

    size_t get_size(file_t f)
    {
        fseek(f, 0, SEEK_END);
        auto size = ftell(f);
        fseek(f, 0, SEEK_SET);

        return size;
    }

    bool read_whole_file(const string_t& path, darray_t<uint8_t>& data)
    {
        auto handle = fopen(path.c_str(),"rb");

        if(handle == nullptr)
        {
            return false;
        }

        auto size = get_size(handle);

        data.resize(size);

        if(fread(data.data(), size, 1, handle) < 1)
        {
            return false;
        }

        return true;
    }
}
