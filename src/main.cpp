// Copyright 2018-2023 Mihail Mladenov
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


#include "application.hpp"

int main()
{
    constexpr const char* app_name = "bpmap";
    constexpr const uint32_t res_x = 1280;
    constexpr const uint32_t res_y = 720;

    bpmap::application_t app(res_x, res_y, app_name);
    app.loop();

    return 0;
}
