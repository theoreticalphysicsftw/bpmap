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
        nk_init_default(&context, nullptr);

        init_font_atlas();
    }

    void gui_t::init_font_atlas()
    {
        nk_font_atlas_init_default(&font_atlas);
        nk_font_atlas_bake(&font_atlas, &font_width, &font_height, NK_FONT_ATLAS_RGBA32);
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
}
