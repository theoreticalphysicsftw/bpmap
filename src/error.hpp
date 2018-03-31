#ifndef ERROR_HPP
#define ERROR_HPP

#include <common.hpp>

namespace bpmap
{
    enum class error_t
    {
        success,
        window_creation_fail,
        instance_creation_fail,
        device_search_fail,
        logical_device_creation_fail,
        get_queue_fail,
        command_pool_creation_fail,
        surface_creation_fail,
        swapchain_creation_fail
    };

    string_t get_error_message(error_t e);
}

#endif // ERROR_HPP
