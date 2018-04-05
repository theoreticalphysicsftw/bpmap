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


#ifndef VULKAN_HPP
#define VULKAN_HPP

#include <common.hpp>
#include <error.hpp>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "../window/window.hpp"

namespace bpmap
{
    static constexpr uint32_t compute_queue = 0;
    static constexpr uint32_t graphics_queue = 1;
    static constexpr uint32_t copy_queue = 2;
    static constexpr uint32_t number_of_queues = 3;

    class vk_buffer_t;
    class vk_image_t;

    class vulkan_t
    {
        VkInstance instance;
        VkDevice device;

        VmaAllocator allocator;

        VkSurfaceKHR surface;
        VkSwapchainKHR swapchain;
        darray_t<VkImage> swapchain_images;
        darray_t<VkImageView> swapchain_image_views;

        VkFormat swapchain_image_format;

        array_t<VkQueue,number_of_queues> queues;
        array_t<VkCommandPool, number_of_queues> command_pools;

        VkPhysicalDevice gpu_device;
        uint32_t graphics_queue_index;
        uint32_t compute_queue_index;
        uint32_t copy_queue_index;

        window_t* window;

        error_t error;

        error_t create_instance();
        error_t find_gpu(VkPhysicalDevice&);

        template <typename Lambda>
        error_t find_queue(VkPhysicalDevice pd, uint32_t& queue_family, Lambda select)
        {
            std::vector<VkQueueFamilyProperties> properties;
            uint32_t count;

            vkGetPhysicalDeviceQueueFamilyProperties(pd, &count, nullptr);
            properties.resize(count);
            vkGetPhysicalDeviceQueueFamilyProperties(pd, &count, properties.data());

            for(uint32_t i = 0; i < count; ++i)
            {
                if(select(properties[i], i))
                {
                    queue_family = i;
                    return error_t::success;
                }
            }

            return error_t::get_queue_fail;
        }

        error_t create_logical_device();
        error_t get_queues();
        error_t create_command_pools();
        error_t create_surface_and_swapchain();
        error_t get_swapchain_images();
        error_t create_allocator();

    public:

        error_t get_status() const { return error;}

        error_t init(window_t&);

        error_t create_buffer(
                               vk_buffer_t& buffer,
                               const VkBufferCreateInfo& bci,
                               const VmaAllocationCreateInfo& aci
                             );
        error_t create_image(
                              vk_image_t& image,
                              const VkImageCreateInfo& ici,
                              const VmaAllocationCreateInfo& aci
                            );

        error_t create_image_view(
                                   VkImageView& image_view,
                                   VkImageViewCreateInfo& ivci
                                 ) const;
        error_t create_sampler(
                                VkSampler& sampler,
                                const VkSamplerCreateInfo& sci
                              ) const;

        void destroy_sampler(VkSampler sampler) const;
        void destroy_image_view(VkImageView image_view) const;

        error_t create_graphics_pipeline(
                                          VkPipeline& pipeline,
                                          const VkGraphicsPipelineCreateInfo& gpci
                                        ) const;

        error_t create_renederpass(
                                    VkRenderPass& render_pass,
                                    const VkRenderPassCreateInfo& rpci
                                  ) const;


        VkFormat get_swapchain_format() const
        {
            return swapchain_image_format;
        }

        ~vulkan_t();
    };


    class vk_buffer_t
    {
        friend class vulkan_t;

        VmaAllocation allocation;
        VmaAllocator allocator;

    public:
        VkBuffer buffer;

        ~vk_buffer_t();
    };

    class vk_image_t
    {
        friend class vulkan_t;

        VmaAllocation allocation;
        VmaAllocator allocator;

    public:
        VkImage image;

        ~vk_image_t();
    };
}
#endif // VULKAN_HPP
