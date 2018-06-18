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

        set_style();

        init_font_atlas();
    }

    void gui_t::init_font_atlas()
    {
        nk_font_atlas_init_default(&font_atlas);
        nk_font_atlas_begin(&font_atlas);
        font_image = nk_font_atlas_bake(&font_atlas, &font_width, &font_height, NK_FONT_ATLAS_RGBA32);
    }

    void gui_t::set_style()
    {
        nk_color table[NK_COLOR_COUNT];
        table[NK_COLOR_TEXT] = nk_rgba(0, 0, 0, 255);
        table[NK_COLOR_WINDOW] = nk_rgba(25, 175, 175, 0);
        table[NK_COLOR_HEADER] = nk_rgba(25, 175, 175, 255);
        table[NK_COLOR_BORDER] = nk_rgba(0, 0, 0, 255);
        table[NK_COLOR_BUTTON] = nk_rgba(25, 185, 195, 255);
        table[NK_COLOR_BUTTON_HOVER] = nk_rgba(188, 222, 222, 255);
        table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(25, 122, 122, 255);
        table[NK_COLOR_TOGGLE] = nk_rgba(150, 150, 150, 255);
        table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(99, 190, 190, 255);
        table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(27, 175, 175, 255);
        table[NK_COLOR_SELECT] = nk_rgba(30, 190, 190, 255);
        table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(175, 175, 175, 255);
        table[NK_COLOR_SLIDER] = nk_rgba(30, 190, 190, 255);
        table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(80, 80, 80, 255);
        table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(70, 70, 70, 255);
        table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(60, 60, 60, 255);
        table[NK_COLOR_PROPERTY] = nk_rgba(25, 175, 175, 255);
        table[NK_COLOR_EDIT] = nk_rgba(20, 150, 150, 255);
        table[NK_COLOR_EDIT_CURSOR] = nk_rgba(0, 0, 0, 255);
        table[NK_COLOR_COMBO] = nk_rgba(25, 175, 175, 255);
        table[NK_COLOR_CHART] = nk_rgba(20, 160, 160, 255);
        table[NK_COLOR_CHART_COLOR] = nk_rgba(10, 45, 45, 255);
        table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba( 255, 0, 0, 255);
        table[NK_COLOR_SCROLLBAR] = nk_rgba(30, 180, 180, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(22, 140, 140, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(80, 210, 210, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(55, 160, 160, 255);
        table[NK_COLOR_TAB_HEADER] = nk_rgba(29, 180, 180, 255);
        nk_style_from_table(&context, table);
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


        data.projection.components[0] = 2.0 / window->get_width();
        data.projection.components[1] = 0.0;
        data.projection.components[2] = 0.0;
        data.projection.components[3] = 0.0;

        data.projection.components[4] = 0.0;
        data.projection.components[5] = 2.0 / window->get_height();
        data.projection.components[6] = 0.0;
        data.projection.components[7] = 0.0;

        data.projection.components[8] = 0.0;
        data.projection.components[9] = 0.0;
        data.projection.components[10] = 1.0;
        data.projection.components[11] = 0.0;

        data.projection.components[12] = -1.0;
        data.projection.components[13] = -1.0;
        data.projection.components[14] = 0.0;
        data.projection.components[15] = 1.0;


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
            call.scissor_horizontal_offset = std::max(cmd->clip_rect.x, 0.0f);
            call.scissor_vertical_offset = std::max(cmd->clip_rect.y, 0.0f);

            calls.push_back(call);

            offset += cmd->elem_count;
        }

        return calls;
    }



    void gui_t::run()
    {
        nk_clear(&context);
        nk_begin(&context, "bpmap", nk_rect(0,0, window->get_width(), window->get_height()), 0);


        nk_layout_row_dynamic(&context, 50, 5);
        if (nk_button_label(&context, "button"))
        {

        }
        nk_layout_row_dynamic(&context, 50, 5);
        if (nk_button_label(&context, "button2"))
        {

        }

        nk_end(&context);
    }
}
