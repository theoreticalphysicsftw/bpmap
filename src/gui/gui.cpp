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


#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION

#include "gui.hpp"

namespace bpmap
{
    gui_t::gui_t()
    {
        nk_buffer_init_default(&commands);

        nk_init_default(&context, nullptr);

        init_font_atlas();
    }

    void gui_t::init_font_atlas()
    {
        nk_font_atlas_init_default(&font_atlas);
        nk_font_atlas_begin(&font_atlas);
        font_image = nk_font_atlas_bake(&font_atlas, &font_width, &font_height, NK_FONT_ATLAS_RGBA32);
    }

    void gui_t::get_input()
    {
        nk_input_begin(&context);

        auto cursor = window->get_mouse_pos();

        nk_input_motion(&context, cursor.first, cursor.second);

        nk_input_button(
                         &context,
                         NK_BUTTON_LEFT,
                         cursor.first,
                         cursor.second,
                         window->mouse_button_pressed(mouse_button_t::left)
                       );

        nk_input_button(
                         &context,
                         NK_BUTTON_RIGHT,
                         cursor.first,
                         cursor.second,
                         window->mouse_button_pressed(mouse_button_t::right)
                       );

        nk_input_button(
                         &context,
                         NK_BUTTON_MIDDLE,
                         cursor.first,
                         cursor.second,
                         window->mouse_button_pressed(mouse_button_t::middle)
                       );

        nk_input_end(&context);
    }

    void gui_t::bind_window(window_t& win)
    {
        window = &win;
    }

    void gui_t::emit_buffers(void* ibuffer, uint32_t ibuffer_size, void* vbuffer, uint32_t vbuffer_size)
    {
        nk_convert_config config = {};

        static const nk_draw_vertex_layout_element vertex_layout[] =
        {
            {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, offsetof(gui_vertex_t, position)},
            {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, offsetof(gui_vertex_t, uv)},
            {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, offsetof(gui_vertex_t, color)},
            {NK_VERTEX_LAYOUT_END}
        };

        config.shape_AA = NK_ANTI_ALIASING_ON;
        config.line_AA = NK_ANTI_ALIASING_ON;
        config.vertex_layout = vertex_layout;
        config.vertex_size = sizeof(gui_vertex_t);
        config.vertex_alignment = NK_ALIGNOF(gui_vertex_t);
        config.circle_segment_count = 22;
        config.curve_segment_count = 22;
        config.arc_segment_count = 22;
        config.global_alpha = 1.0f;
        config.null = null_texture;


        nk_buffer indices;
        nk_buffer vertices;

        nk_buffer_free(&commands);
        nk_buffer_init_default(&commands);

        nk_buffer_init_fixed(&vertices,vbuffer, vbuffer_size);
        nk_buffer_init_fixed(&indices,ibuffer, ibuffer_size);

        nk_convert(&context, &commands, &vertices, &indices, &config);
    }

    void gui_t::finalize_font_atlas()
    {
        nk_font_atlas_end(&font_atlas, nk_handle_ptr(nullptr), &null_texture);
        nk_style_set_font(&context, &font_atlas.default_font->handle);
    }

    gui_data_t gui_t::get_gui_data()
    {
        gui_data_t data;

        memset(&data, 0, sizeof(data));

        data.projection.components[0] = 1.0 / window->get_width();
        data.projection.components[5] = 1.0 / window->get_height();
        data.projection.components[10] = -1.0;
        data.projection.components[16] = 1.0;

        return data;
    }

    darray_t<draw_call_t> gui_t::emit_draw_calls()
    {
        static constexpr uint32_t init_calls_size = 1 << 12;
        darray_t<draw_call_t> calls;
        calls.reserve(init_calls_size);

        auto offset = 0;
        const nk_draw_command* cmd;
        nk_draw_foreach(cmd, &context, &commands)
        {
            draw_call_t call = {};
            call.elements = cmd->elem_count;
            call.offset = offset;
            call.scissor_height = cmd->clip_rect.h;
            call.scissor_width = cmd->clip_rect.h;
            call.scissor_horizontal_offset = cmd->clip_rect.x;
            call.scissor_vertical_offset = cmd->clip_rect.y;

            calls.push_back(call);

            offset += cmd->elem_count;
        }

        return calls;
    }



    void gui_t::run()
    {
        nk_begin(&context, "bpmap", nk_rect(0,0, window->get_width(), window->get_height()), 0);

        nk_clear(&context);

        if(nk_button_label(&context, "test"))
        {

        }

        nk_end(&context);
    }
}
