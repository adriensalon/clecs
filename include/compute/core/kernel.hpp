#pragma once

#include <compute/core/buffer.hpp>
#include <compute/core/context.hpp>

#include <string>

namespace compute {

/// @brief Represents a compiled OpenCL kernel ready for execution on the device.
/// The `kernel` type wraps the lifecycle and invocation of OpenCL compute kernels.
/// It compiles OpenCL C source at runtime, binds device buffers as arguments, and
/// dispatches execution over a specified workgroup range. Kernels are non-copyable
/// but movable.
struct kernel {

    kernel(const kernel& other) = delete;
    kernel& operator=(const kernel& other) = delete;
    kernel(kernel&& other) noexcept;
    kernel& operator=(kernel&& other) noexcept;
    ~kernel();

    /// @brief Compiles a kernel from OpenCL source.
    /// Constructs and builds an OpenCL program from the provided source string,
    /// and extracts the named kernel function. The kernel is compiled specifically
    /// for the device associated with the provided context.
    /// @param ctx The execution context and device this kernel is bound to.
    /// @param str OpenCL C source code as a string.
    /// @param name The name of the kernel function to extract and run.
    kernel(const context& ctx, const std::string& str, const std::string& name);

    /// @brief Sets a kernel argument using a device buffer.
    /// Binds a `buffer<value_t>` as a kernel argument at the specified index.
    /// @tparam value_t Type of the buffer data.
    /// @param idx Index of the kernel argument.
    /// @param buf Reference to the device buffer.
    template <typename value_t>
    void set_arg(const std::size_t idx, buffer<value_t>& buf);

    /// @brief Sets a kernel argument using a device array buffer.
    /// Binds an `array_buffer<value_t>` as a kernel argument at the specified index.
    /// @tparam value_t Type of the buffer elements.
    /// @param idx Index of the kernel argument.
    /// @param buf Reference to the array buffer.
    template <typename value_t>
    void set_arg(const std::size_t idx, array_buffer<value_t>& buf);

    /// @brief Launches the kernel with the specified global work size.
    /// Executes the kernel on the associated device using the provided
    /// global work dimensions. The kernel must be fully configured with all
    /// arguments set prior to execution.
    /// @param wsz Vector of global work sizes for each dimension (e.g., 1D, 2D, 3D).
    std::future<void> run(const std::vector<std::size_t>& wsz);

private:
    cl_device_id _device;
    cl_context _context;
    cl_command_queue _command_queue;
    cl_program _program;
    cl_kernel _kernel;
};

}

#include "kernel.inl"
