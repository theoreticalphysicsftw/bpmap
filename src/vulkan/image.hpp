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

#ifndef VULKAN_IMAGE_HPP
#define VULKAN_IMAGE_HPP


namespace bpmap
{
    enum class vk_image_format_t
    {
        rgba8u = 0,
        rgba32f
    };

    enum class vk_image_tiling_t
    {
        optimal = 0,
        linear,
    };

    static constexpr uint32_t vk_usage_color = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    static constexpr uint32_t vk_usage_sampled = VK_IMAGE_USAGE_SAMPLED_BIT;
    static constexpr uint32_t vk_usage_storage = VK_IMAGE_USAGE_STORAGE_BIT;
    static constexpr uint32_t vk_usage_transfer_dst = VK_IMAGE_USAGE_TRANSFER_DST_BIT;


    struct vk_image_desc_t
    {
        uint32_t width = 0;
        uint32_t height = 0;
        vk_image_format_t format = vk_image_format_t::rgba32f;
        vk_image_tiling_t tiling = vk_image_tiling_t::optimal;
        bool on_gpu = true;
        uint32_t usage = vk_usage_sampled | vk_usage_storage;
        uint32_t samples = 1;
        uint32_t mips = 1;
        uint32_t layers = 1;
    };

    class vk_image_t
    {
        const vk_device_t* dev;
        bool_t wrapped;
        VmaAllocation allocation;
        VkImage image;
        VkImageView view;
        
        vk_image_desc_t info;

        vk_image_t(const vk_image_t&) = delete;
        vk_image_t& operator=(const vk_image_t&) = delete;

    public:
        error_t create(
                        const vk_device_t& device,
                        const vk_image_desc_t& desc
                      );

        VkImage get_image() const { return image; }
        VkImageView get_view() const { return view; }
        vk_image_t();
        ~vk_image_t();
    };
}
#endif
