#ifndef GUI_RENDERER_HPP
#define GUI_RENDERER_HPP

#include "vulkan.hpp"
#include "gui/gui.hpp"

namespace bpmap
{
    class gui_renderer_t
    {
        vk_image_t font_image;

        VkImageView font_view;
        VkSampler font_sampler;

        vk_buffer_t vertex_buffer;
        vk_buffer_t index_buffer;

        const gui_t* gui;
        const vulkan_t* vulkan;

        void setup_font_texture();

    public:

        void bind_gui(const gui_t& gui);
        void bind_vulkan(const vulkan_t& vulkan);

        error_t init();

        ~gui_renderer_t();
    };
}

#endif // GUI_RENDERER_HPP
