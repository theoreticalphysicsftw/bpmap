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


namespace bpmap
{
    class vk_fence_t;
    class vk_semaphore_t;
    class vk_command_pool_t;

    class vk_device_t
    {
        VkInstance instance;
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

        VkPhysicalDevice gpu_device;
        uint32_t queue_index;

        window_t* window;

        error_t create_instance(const string_t& app_name);
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
        error_t validate_surface_support();
        error_t pick_surface_format();
        error_t pick_present_mode();
        error_t create_surface_and_swapchain();
        error_t get_swapchain_images();
        error_t create_allocator();

    public:

        error_t init(window_t&);

        error_t create_pipeline_layout(
                                        VkPipelineLayout& layout,
                                        const VkPipelineLayoutCreateInfo& plci
                                      ) const;

        void destroy_pipeline_layout(VkPipelineLayout layout) const;

        error_t create_descriptor_set_layout(
                                              VkDescriptorSetLayout& layout,
                                              const VkDescriptorSetLayoutCreateInfo& dslci
                                            ) const;

        void destroy_descriptor_set_layout(VkDescriptorSetLayout layout) const;

        error_t create_descriptor_pool(
                                        VkDescriptorPool& pool,
                                        const VkDescriptorPoolCreateInfo& dpci
                                      ) const;

        void destroy_descriptor_pool(VkDescriptorPool pool) const;

        error_t allocate_descriptor_set(
                                         VkDescriptorSet& set,
                                         const VkDescriptorSetAllocateInfo& dsai
                                       ) const;

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

        error_t create_command_pool(vk_command_pool_t& pool) const;

        error_t create_command_buffers(
                                        VkCommandBuffer* buffers,
                                        const VkCommandBufferAllocateInfo& cbai
                                      ) const;

        error_t submit_work(const VkSubmitInfo& submit_info, const vk_fence_t* fence) const;
        error_t wait_idle() const;

        VkFormat get_swapchain_format() const
        {
            return swapchain_image_format;
        }

        void update_descriptor_sets(const VkWriteDescriptorSet* writes, uint32_t count) const;

        error_t get_next_swapchain_image(uint32_t&, const vk_semaphore_t*) const;
        error_t present_on_screen(uint32_t index, const vk_semaphore_t* wait_semaphore) const;

        ~vk_device_t();

        VkDevice get_device() const { return device; }
        VmaAllocator get_allocator() const { return allocator; }
    };


    class vk_command_pool_t
    {
        friend class vk_device_t;

        VkDevice device;

    public:
        VkCommandPool pool;

        vk_command_pool_t();
        ~vk_command_pool_t();
    };

}
#endif // VULKAN_HPP
