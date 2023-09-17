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


#ifndef VULKAN_DEVICE_HPP
#define VULKAN_DEVICE_HPP

#include "../window/window.hpp"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <bitmap_tree.hpp>


namespace bpmap::vk
{
    class fence_t;
    class semaphore_t;
    class command_pool_t;
    class image_t;
    class buffer_t;
    class sampler_t;

    struct device_desc_t
    {
        uint32_t max_bindless_buffers = 4096 * 64;
        uint32_t max_bindless_images = 4096 * 64;
        uint32_t max_bindless_samplers = 4096 * 64;
        uint32_t max_push_constants = 32;
    };

    static constexpr uint32_t INVALID_SLOT = ~uint32_t(0);

    class device_t
    {
        VkInstance instance;
        VkPhysicalDevice gpu_device;
        VkDevice device;

        VmaAllocator allocator;

        VkSurfaceKHR surface;
        VkSwapchainKHR swapchain;
        darray_t<VkImage> swapchain_images;
        darray_t<VkImageView> swapchain_image_views;
        VkFormat swapchain_image_format;
        VkColorSpaceKHR swapchain_color_space;
        VkPresentModeKHR swapchain_present_mode;

        VkQueue queue;
        uint32_t queue_index;

        window_t* window;

        error_t create_instance(const string_t& app_name);
        error_t find_gpu(VkPhysicalDevice&);

        template <typename Lambda>
        error_t find_queue(VkPhysicalDevice pd, uint32_t& queue_family, Lambda select);

        error_t create_logical_device();
        error_t get_queues();
        error_t validate_surface_support();
        error_t pick_surface_format();
        error_t pick_present_mode();
        error_t create_surface_and_swapchain();
        error_t get_swapchain_images();
        error_t create_allocator();

        // Bindless system;
        mutable bmt::tree_t<uint64_t> image_slots;
        mutable bmt::tree_t<uint64_t> sampler_slots;
        mutable bmt::tree_t<uint64_t> buffer_slots;
        VkDescriptorPool bindless_pool;
        VkDescriptorSetLayout bindless_layout;
        VkDescriptorSet bindless_set;
        VkPushConstantRange push_range;

        error_t init_bindless_system(const device_desc_t& desc);
        void destroy_bindless_system();

    public:

        VkDescriptorSetLayout get_bindless_layout() const { return bindless_layout; }
        VkDescriptorSet get_bindless_set() const { return bindless_set; }
        const VkPushConstantRange& get_push_range() const { return push_range; }

        error_t init(window_t&, const device_desc_t& desc = device_desc_t());

        uint32_t bind(const image_t* image) const;
        uint32_t bind(const buffer_t* buffer) const;
        uint32_t bind(const sampler_t* sampler) const;

        error_t create_pipeline_layout(
                                        VkPipelineLayout& layout,
                                        const VkPipelineLayoutCreateInfo& plci
                                      ) const;

        void destroy_pipeline_layout(VkPipelineLayout layout) const;


        error_t create_graphics_pipeline(
                                          VkPipeline& pipeline,
                                          const VkGraphicsPipelineCreateInfo& gpci
                                        ) const;

        error_t create_compute_pipelines(
                                          darray_t<VkPipeline>& pipeline,
                                          const darray_t<VkComputePipelineCreateInfo>& cpci
                                        ) const;

        void destroy_pipeline(VkPipeline) const;

        error_t create_renederpass(
                                    VkRenderPass& render_pass,
                                    const VkRenderPassCreateInfo& rpci
                                  ) const;

        void destroy_render_pass(VkRenderPass render_pass) const;

        error_t create_framebuffers(
                                     darray_t<VkFramebuffer>& framebuffers,
                                     VkFramebufferCreateInfo& fbci
                                   ) const ;

        void destroy_framebuffers(const darray_t<VkFramebuffer>& framebuffers) const;

        error_t create_command_pool(command_pool_t& pool) const;

        error_t create_command_buffers(
                                        VkCommandBuffer* buffers,
                                        const VkCommandBufferAllocateInfo& cbai
                                      ) const;

        error_t submit_work(const VkSubmitInfo& submit_info, const fence_t* fence) const;
        error_t wait_idle() const;

        VkFormat get_swapchain_format() const
        {
            return swapchain_image_format;
        }

        error_t get_next_swapchain_image(uint32_t&, const semaphore_t*) const;
        error_t present_on_screen(uint32_t index, const semaphore_t* wait_semaphore) const;

        ~device_t();

        VkDevice get_device() const { return device; }
        VmaAllocator get_allocator() const { return allocator; }
    };

    template <typename Lambda>
    error_t device_t::find_queue(VkPhysicalDevice pd, uint32_t& queue_family, Lambda select)
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

    class command_pool_t
    {
        friend class device_t;

        VkDevice device;

    public:
        VkCommandPool pool;

        command_pool_t();
        ~command_pool_t();
    };

}
#endif
