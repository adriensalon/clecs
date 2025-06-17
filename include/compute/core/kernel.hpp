#pragma once

#include <compute/core/buffer.hpp>
#include <compute/core/context.hpp>

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

    /// @brief 
    /// @tparam value_t 
    /// @param idx 
    /// @param buf 
    template <typename value_t>
    void set_arg(const std::size_t idx, buffer<value_t>& buf);

    /// @brief 
    /// @tparam value_t 
    /// @param idx 
    /// @param buf 
    template <typename value_t>
    void set_arg(const std::size_t idx, array_buffer<value_t>& buf);

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

#include "kernel.inl"
