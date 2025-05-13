#pragma once

#include <compute/core/device.hpp>

namespace compute {

/// @brief
struct context {

    context(const context& other) = delete;
    context& operator=(const context& other) = delete;
    context(context&& other) noexcept;
    context& operator=(context&& other) noexcept;
    ~context();

    /// @brief
    /// @param device
    context(const device& dev, const std::vector<cl_context_properties>& props = {});

private:
    cl_device_id _device;
    cl_context _context;
    cl_command_queue _queue;
    template <typename value_t> friend struct buffer;
    template <typename value_t> friend struct array_buffer;
    friend struct kernel;
};

}
