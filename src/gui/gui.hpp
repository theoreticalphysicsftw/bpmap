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

namespace bpmap
{
    class gui_t
    {
        window_t * window;

        nk_context context;
        nk_font_atlas font_atlas;

        const void* font_image;
        int32_t font_height;
        int32_t font_width;

        void init_font_atlas();
        void get_input();

    public:
        gui_t();
        void bind_window(window_t& window);
    };
}
#endif // GUI_HPP
