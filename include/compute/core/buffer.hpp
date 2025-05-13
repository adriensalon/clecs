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
    buffer(buffer&& other) noexcept
        : _mem(other._mem)
        , _queue(other._queue)
    {
        other._mem = nullptr;
        other._queue = nullptr;
    }
    buffer& operator=(buffer&& other) noexcept
    {
        if (this != &other) {
            release();
            _mem = other._mem;
            _queue = other._queue;
            other._mem = nullptr;
            other._queue = nullptr;
        }
        return *this;
    }
    ~buffer()
    {
        release();
    }

    /// @brief
    /// @param ctx
    buffer(const context& ctx, cl_mem_flags flags = CL_MEM_READ_WRITE)
        : _queue(ctx._queue)
    {
        cl_int err = 0;
        _mem = clCreateBuffer(ctx._context, flags, sizeof(value_t), nullptr, &err);
        if (err != CL_SUCCESS)
            throw std::runtime_error("Failed to create OpenCL buffer");
    }

    void set(const value_t& val)
    {
        cl_int err = clEnqueueWriteBuffer(_queue, _mem, CL_TRUE, 0, sizeof(value_t), &val, 0, nullptr, nullptr);
        if (err != CL_SUCCESS)
            throw std::runtime_error("Failed to write to OpenCL buffer");
    }

    std::future<value_t> fetch()
    {
        return std::async(std::launch::async, [this]() {
            value_t val;
            cl_int err = clEnqueueReadBuffer(_queue, _mem, CL_TRUE, 0, sizeof(value_t), &val, 0, nullptr, nullptr);
            if (err != CL_SUCCESS)
                throw std::runtime_error("Failed to read from OpenCL buffer");
            return val;
        });
    }

private:
    cl_mem _mem;
    cl_command_queue _queue;
    friend struct kernel;

    void release()
    {
        if (_mem)
            clReleaseMemObject(_mem);
    }
};

template <typename value_t>
struct array_buffer {

    array_buffer(const array_buffer& other) = delete;
    array_buffer& operator=(const array_buffer& other) = delete;
    array_buffer(array_buffer&& other) noexcept
        : _size(other._size)
        , _mem(other._mem)
        , _queue(other._queue)
    {
        other._size = 0;
        other._mem = nullptr;
        other._queue = nullptr;
    }

    array_buffer& operator=(array_buffer&& other) noexcept
    {
        if (this != &other) {
            release();
            _size = other._size;
            _mem = other._mem;
            _queue = other._queue;
            other._size = 0;
            other._mem = nullptr;
            other._queue = nullptr;
        }
        return *this;
    }

    ~array_buffer()
    {
        release();
    }

    array_buffer(const context& ctx, const std::size_t sz, cl_mem_flags flags = CL_MEM_READ_WRITE)
        : _size(sz)
        , _queue(ctx._queue)
    {
        cl_int err = 0;
        _mem = clCreateBuffer(ctx._context, flags, sz * sizeof(value_t), nullptr, &err);
        if (err != CL_SUCCESS)
            throw std::runtime_error("Failed to create OpenCL array buffer");
    }

    void set(std::size_t idx, const value_t& val)
    {
        if (idx >= _size)
            throw std::out_of_range("Index out of bounds");
        cl_int err = clEnqueueWriteBuffer(_queue, _mem, CL_TRUE, idx * sizeof(value_t), sizeof(value_t), &val, 0, nullptr, nullptr);
        if (err != CL_SUCCESS)
            throw std::runtime_error("Failed to write element to array buffer");
    }

    // void set(std::size_t start, const std::vector<value_t>& vals)
    // {
    //     const size_t offset_bytes = start * sizeof(value_t);
    //     const size_t size_bytes = vals.size() * sizeof(value_t);

    //     cl_int err = clEnqueueWriteBuffer(
    //         _queue, // Command queue
    //         _mem, // Memory object
    //         CL_TRUE, // Blocking write
    //         offset_bytes, // Offset in buffer (in bytes)
    //         size_bytes, // Size to write (in bytes)
    //         vals.data(), // Source data
    //         0, nullptr, nullptr);

    //     if (err != CL_SUCCESS) {
    //         throw std::runtime_error("Failed to write vector to buffer at offset " + std::to_string(start));
    //     }
    // }

    void set(const std::vector<value_t>& vals)
    {
        if (vals.size() > _size)
            throw std::out_of_range("Input vector size exceeds buffer size");
        cl_int err = clEnqueueWriteBuffer(
            _queue,
            _mem,
            CL_TRUE, // blocking write
            0,
            vals.size() * sizeof(value_t),
            vals.data(),
            0,
            nullptr,
            nullptr);
        if (err != CL_SUCCESS)
            throw std::runtime_error("Failed to write vector to array buffer");
    }

    [[nodiscard]] std::future<value_t> fetch(std::size_t idx)
    {
        return std::async(std::launch::async, [this, idx]() {
            if (idx >= _size)
                throw std::out_of_range("Index out of bounds");
            value_t val;
            cl_int err = clEnqueueReadBuffer(_queue, _mem, CL_TRUE, idx * sizeof(value_t), sizeof(value_t), &val, 0, nullptr, nullptr);
            if (err != CL_SUCCESS)
                throw std::runtime_error("Failed to read element");
            return val;
        });
    }

    // [[nodiscard]] std::future<std::vector<value_t>> fetch(std::size_t start, std::size_t end)
    // {
    //     return std::async(std::launch::async, [this, start, end]() {
    //         if (start >= end || end > _size)
    //             throw std::out_of_range("Invalid read range");
    //         std::vector<value_t> result(end - start);
    //         cl_int err = clEnqueueReadBuffer(_queue, _mem, CL_TRUE, start * sizeof(value_t),
    //             result.size() * sizeof(value_t), result.data(), 0, nullptr, nullptr);
    //         if (err != CL_SUCCESS)
    //             throw std::runtime_error("Failed to read array range");
    //         return result;
    //     });
    // }

    [[nodiscard]] std::future<std::vector<value_t>> fetch()
    {
        return std::async(std::launch::async, [this]() {
            std::vector<value_t> result(_size);
            cl_int err = clEnqueueReadBuffer(
                _queue,
                _mem,
                CL_TRUE, // blocking read
                0,
                _size * sizeof(value_t),
                result.data(),
                0,
                nullptr,
                nullptr);
            if (err != CL_SUCCESS)
                throw std::runtime_error("Failed to read array buffer");
            return result;
        });
    }

    [[nodiscard]] std::size_t get_size() const
    {
        return _size;
    }

private:
    std::size_t _size;
    cl_mem _mem;
    cl_command_queue _queue;
    friend struct kernel;

    void release()
    {
        if (_mem)
            clReleaseMemObject(_mem);
    }
};
}