#pragma once

#include <compute/core/context.hpp>

#include <future>

namespace compute {

/// @brief
/// @tparam value_t
template <typename value_t>
struct buffer {

    buffer(const buffer& other) = delete;
    buffer& operator=(const buffer& other) = delete;
    buffer(buffer&& other) noexcept;
    buffer& operator=(buffer&& other) noexcept;
    ~buffer();

    /// @brief
    /// @param ctx
    /// @param flags
    buffer(const context& ctx, cl_mem_flags flags = CL_MEM_READ_WRITE);

    /// @brief
    /// @param val
    void set(const value_t& val);

    /// @brief
    /// @return
    [[nodiscard]] std::future<value_t> fetch();

private:
    cl_mem _mem;
    cl_command_queue _queue;
    friend struct kernel;

    void _release();
};

/// @brief
/// @tparam value_t
template <typename value_t>
struct array_buffer {

    array_buffer(const array_buffer& other) = delete;
    array_buffer& operator=(const array_buffer& other) = delete;
    array_buffer(array_buffer&& other) noexcept;
    array_buffer& operator=(array_buffer&& other) noexcept;
    ~array_buffer();

    /// @brief 
    /// @param ctx 
    /// @param sz 
    /// @param flags 
    array_buffer(const context& ctx, const std::size_t sz, cl_mem_flags flags = CL_MEM_READ_WRITE);

    /// @brief 
    /// @param idx 
    /// @param val 
    void set(std::size_t idx, const value_t& val);

    /// @brief 
    /// @param vals 
    void set(const std::vector<value_t>& vals);

    /// @brief 
    /// @param idx 
    /// @return 
    [[nodiscard]] std::future<value_t> fetch(std::size_t idx);

    /// @brief 
    /// @return 
    [[nodiscard]] std::future<std::vector<value_t>> fetch();

    /// @brief 
    /// @return 
    [[nodiscard]] std::size_t get_size() const;

private:
    std::size_t _size;
    cl_mem _mem;
    cl_command_queue _queue;
    friend struct kernel;

    void _release();
};
}

#include "buffer.inl"
