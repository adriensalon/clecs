#pragma once

#include <compute/core/buffer.hpp>
#include <compute/core/context.hpp>
#include <compute/sharing/shared_buffer.hpp>

#include <string>

namespace compute {

/// @brief
struct kernel {

    kernel(const kernel& other) = delete;
    kernel& operator=(const kernel& other) = delete;
    kernel(kernel&& other) noexcept;
    kernel& operator=(kernel&& other) noexcept;
    ~kernel();

    /// @brief
    /// @param ctx
    /// @param code
    kernel(const context& ctx, const std::string& str, const std::string& name);

    // + FROM SHARED CTX !

    template <typename value_t>
    void set_arg(const std::size_t idx, buffer<value_t>& buf)
    {
        cl_int err = clSetKernelArg(_kernel, static_cast<cl_uint>(idx), sizeof(cl_mem), &(buf._mem));
        if (err != CL_SUCCESS) {
            throw std::runtime_error("Failed to set buffer kernel argument at index " + std::to_string(idx));
        }
    }

    template <typename value_t>
    void set_arg(const std::size_t idx, array_buffer<value_t>& buf)
    {
        cl_int err = clSetKernelArg(_kernel, static_cast<cl_uint>(idx), sizeof(cl_mem), &(buf._mem));
        if (err != CL_SUCCESS) {
            throw std::runtime_error("Failed to set array_buffer kernel argument at index " + std::to_string(idx));
        }
    }

    void set_arg(const std::size_t idx, shared_buffer& buf)
    {
        // todo
    }

    /// @brief
    void run(const std::vector<std::size_t>& wsz);

private:
    cl_device_id _device;
    cl_context _context;
    cl_command_queue _command_queue;
    cl_program _program;
    cl_kernel _kernel;
};

}