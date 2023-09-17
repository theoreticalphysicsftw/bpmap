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


#ifndef GUI_HPP
#define GUI_HPP

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

#include <nuklear.h>
#include <common.hpp>

#include "window/window.hpp"
#include "core/algebra.hpp"

namespace bpmap
{
    struct gui_data_t
    {
        transform3d_embedded_t projection;
        float_t render_a;
        float_t render_gamma;
        uint32_t font_texture_id;
        uint32_t font_sampler_id;
    };

    struct draw_call_t
    {
        float_t scissor_width;
        float_t scissor_height;
        float_t scissor_horizontal_offset;
        float_t scissor_vertical_offset;
        uint16_t elements;
        uint16_t offset;
    };

    class gui_t
    {
        window_t* window;

        nk_context context;
        nk_font_atlas font_atlas;
        nk_draw_null_texture null_texture;
        nk_buffer commands;

        const void* font_image;
        int32_t font_height;
        int32_t font_width;

        void init_font_atlas();

        void set_style();

        float_t render_a = 1.0;
        float_t render_gamma = 1.0;

        bool_t data_changed = false;

    public:
        gui_t(window_t& window);

        int32_t get_font_height() const {return font_height;}
        int32_t get_font_width() const {return font_width;}
        const void* get_raw_font() const {return font_image;}

        void emit_buffers(
                           void* ibuffer,
                           uint32_t ibuffer_size,
                           void* vbuffer,
                           uint32_t vbuffer_size
                         );

        void finalize_font_atlas() ;

        gui_data_t get_gui_data();

        darray_t<draw_call_t> emit_draw_calls();

        size_t get_width() const {return window->get_width();}
        size_t get_height() const {return window->get_height();}

        bool_t gui_data_changed() {return data_changed;}

        void get_input();
        void run();
    };

    struct gui_vertex_t
    {
        point2d_t position;
        point2d_t uv; // Parametrization coordinates
        color4d_t color;
    };
}
#endif // GUI_HPP
