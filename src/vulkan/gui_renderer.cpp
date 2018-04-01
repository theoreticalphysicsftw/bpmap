#include "gui_renderer.hpp"

namespace bpmap
{
    void gui_renderer_t::setup_font_texture()
    {

    }


    void gui_renderer_t::bind_gui(const gui_t &g)
    {
        gui = &g;
    }

    void gui_renderer_t::bind_vulkan(const vulkan_t &vk)
    {
        vulkan = &vk;
    }

    error_t gui_renderer_t::init()
    {
        setup_font_texture();

        return error_t::success;
    }

    gui_renderer_t::~gui_renderer_t()
    {
        vulkan->destroy_image_view(font_view);
    }
}
