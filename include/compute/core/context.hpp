#pragma once

#include <compute/core/device.hpp>

namespace compute {

/// @brief Represents an OpenCL execution context bound to a specific device.
/// The `context` is responsible for managing the OpenCL context and associated command queue
/// used for executing kernels and managing memory buffers. It is tightly coupled with a specific
/// OpenCL device. Contexts are non-copyable but movable. They manage ownership of the underlying
/// OpenCL context and command queue, and clean them up automatically on destruction.
struct context {

    context(const context& other) = delete;
    context& operator=(const context& other) = delete;
    context(context&& other) noexcept;
    context& operator=(context&& other) noexcept;
    ~context();

    /// @brief Constructs a new context bound to a given compute device.
    /// This sets up the OpenCL context and command queue that ECS systems will use
    /// to interact with device memory and launch compute kernels.
    /// @param dev The `device` instance this context will be associated with.
    /// @param props Optional additional OpenCL context properties (can be empty).
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
