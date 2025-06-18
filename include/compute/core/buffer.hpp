#pragma once

#include <compute/core/context.hpp>

#include <future>

namespace compute {

/// @brief Represents a single device-resident value accessible via OpenCL.
/// `buffer<value_t>` provides a thin abstraction over an OpenCL memory object
/// holding a single instance of `value_t`. It is created inside a given `context`
/// and supports asynchronous fetch back to host memory.
/// @tparam value_t The type of the value stored in device memory.
template <typename value_t>
struct buffer {

    buffer(const buffer& other) = delete;
    buffer& operator=(const buffer& other) = delete;
    buffer(buffer&& other) noexcept;
    buffer& operator=(buffer&& other) noexcept;
    ~buffer();

    /// @brief Constructs a device buffer for a single value.
    /// @param ctx The compute context this buffer will reside in.
    /// @param flags OpenCL memory access flags (e.g., `CL_MEM_READ_WRITE`).
    buffer(const context& ctx, cl_mem_flags flags = CL_MEM_READ_WRITE);

    /// @brief Sets the value on the device from host memory.
    /// This writes the specified value to device memory asynchronously.
    /// @param val The value to write to the buffer.
    void set(const value_t& val); // -----------------------> go return std::future<void> async

    /// @brief Asynchronously fetches the value from device memory.
    /// Returns a future that will contain the host copy of the value once
    /// the transfer completes.
    /// @return A `std::future<value_t>` that resolves with the current value.
    [[nodiscard]] std::future<value_t> fetch();

private:
    cl_mem _mem;
    cl_command_queue _queue;
    friend struct kernel;
    void _release();
};

/// @brief Represents an array of device-resident values in contiguous device memory.
/// `array_buffer<value_t>` provides a resizable OpenCL buffer of elements of type `value_t`,
/// designed for use in processing where systems operate over arrays
/// of components directly in device memory. Like `buffer<value_t>`, values can be set or
/// fetched from the host side asynchronously.
/// @tparam value_t The type of each element in the buffer.
template <typename value_t>
struct array_buffer {

    array_buffer(const array_buffer& other) = delete;
    array_buffer& operator=(const array_buffer& other) = delete;
    array_buffer(array_buffer&& other) noexcept;
    array_buffer& operator=(array_buffer&& other) noexcept;
    ~array_buffer();

    /// @brief Constructs an array buffer with a fixed size.
    /// @param ctx The compute context this buffer will reside in.
    /// @param sz Number of elements to allocate.
    /// @param flags OpenCL memory flags (default: `CL_MEM_READ_WRITE`).
    array_buffer(const context& ctx, const std::size_t sz, cl_mem_flags flags = CL_MEM_READ_WRITE);

    /// @brief Sets a single element in the device buffer.
    /// Throws std::out_of_range exception if index is greater than the buffer size.
    /// @param idx Index of the element to update.
    /// @param val The new value to write at the given index.
    void set(std::size_t idx, const value_t& val); // -----------------------> go return std::future<void> async

    /// @brief Sets multiple values in the device buffer from host memory.
    /// Replaces the beginning of the buffer with the provided values.
    /// If fewer values are provided than the buffer size, only those are updated.
    /// Throws std::out_of_range exception if more values are provided than the buffer size.
    /// @param vals A vector of values to copy into the buffer.
    void set(const std::vector<value_t>& vals); // -----------------------> go return std::future<void> async

    /// @brief Asynchronously fetches a single element from device memory.
    /// Throws std::out_of_range exception if index is greater than the buffer size.
    /// @param idx Index of the element to fetch.
    /// @return A future resolving to the value at the specified index.
    [[nodiscard]] std::future<value_t> fetch(std::size_t idx);

    /// @brief Asynchronously fetches the entire array from the device.
    /// @return A future resolving to a `std::vector` containing all elements.
    [[nodiscard]] std::future<std::vector<value_t>> fetch();

    /// @brief Returns the number of elements in the buffer.
    /// @return The current size of the array buffer.
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
