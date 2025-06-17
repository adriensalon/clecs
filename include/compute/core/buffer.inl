namespace compute {

template <typename value_t>
buffer<value_t>::buffer(buffer&& other) noexcept
    : _mem(other._mem)
    , _queue(other._queue)
{
    other._mem = nullptr;
    other._queue = nullptr;
}

template <typename value_t>
buffer<value_t>& buffer<value_t>::operator=(buffer&& other) noexcept
{
    if (this != &other) {
        _release();
        _mem = other._mem;
        _queue = other._queue;
        other._mem = nullptr;
        other._queue = nullptr;
    }
    return *this;
}

template <typename value_t>
buffer<value_t>::~buffer()
{
    _release();
}

template <typename value_t>
buffer<value_t>::buffer(const context& ctx, cl_mem_flags flags)
    : _queue(ctx._queue)
{
    auto _err = 0;
    _mem = clCreateBuffer(ctx._context, flags, sizeof(value_t), nullptr, &_err);
    if (_err != CL_SUCCESS) {
        throw std::runtime_error("Failed to create OpenCL buffer");
    }
}

template <typename value_t>
void buffer<value_t>::set(const value_t& val)
{
    auto _err = clEnqueueWriteBuffer(_queue, _mem, CL_TRUE, 0, sizeof(value_t), &val, 0, nullptr, nullptr);
    if (_err != CL_SUCCESS) {
        throw std::runtime_error("Failed to write to OpenCL buffer");
    }
}

template <typename value_t>
std::future<value_t> buffer<value_t>::fetch()
{
    return std::async(std::launch::async, [this]() {
        auto _val = value_t {};
        auto _err = clEnqueueReadBuffer(_queue, _mem, CL_TRUE, 0, sizeof(value_t), &_val, 0, nullptr, nullptr);
        if (_err != CL_SUCCESS) {
            throw std::runtime_error("Failed to read from OpenCL buffer");
        }
        return _val;
    });
}

template <typename value_t>
void buffer<value_t>::_release()
{
    if (_mem) {
        clReleaseMemObject(_mem);
    }
}

template <typename value_t>
array_buffer<value_t>::array_buffer(array_buffer&& other) noexcept
    : _size(other._size)
    , _mem(other._mem)
    , _queue(other._queue)
{
    other._size = 0;
    other._mem = nullptr;
    other._queue = nullptr;
}

template <typename value_t>
array_buffer<value_t>& array_buffer<value_t>::operator=(array_buffer&& other) noexcept
{
    if (this != &other) {
        _release();
        _size = other._size;
        _mem = other._mem;
        _queue = other._queue;
        other._size = 0;
        other._mem = nullptr;
        other._queue = nullptr;
    }
    return *this;
}

template <typename value_t>
array_buffer<value_t>::~array_buffer()
{
    _release();
}

template <typename value_t>
array_buffer<value_t>::array_buffer(const context& ctx, const std::size_t sz, cl_mem_flags flags)
    : _size(sz)
    , _queue(ctx._queue)
{
    auto _err = 0;
    _mem = clCreateBuffer(ctx._context, flags, sz * sizeof(value_t), nullptr, &_err);
    if (_err != CL_SUCCESS) {
        throw std::runtime_error("Failed to create OpenCL array buffer");
    }
}

template <typename value_t>
void array_buffer<value_t>::set(std::size_t idx, const value_t& val)
{
    if (idx >= _size) {
        throw std::out_of_range("Index out of bounds");
    }
    auto _err = clEnqueueWriteBuffer(_queue, _mem, CL_TRUE, idx * sizeof(value_t), sizeof(value_t), &val, 0, nullptr, nullptr);
    if (_err != CL_SUCCESS) {
        throw std::runtime_error("Failed to write element to array buffer");
    }
}

template <typename value_t>
void array_buffer<value_t>::set(const std::vector<value_t>& vals)
{
    if (vals.size() > _size) {
        throw std::out_of_range("Input vector size exceeds buffer size");
    }
    auto _err = clEnqueueWriteBuffer(_queue, _mem, CL_TRUE, 0, vals.size() * sizeof(value_t), vals.data(), 0, nullptr, nullptr);
    if (_err != CL_SUCCESS) {
        throw std::runtime_error("Failed to write vector to array buffer");
    }
}

template <typename value_t>
std::future<value_t> array_buffer<value_t>::fetch(std::size_t idx)
{
    return std::async(std::launch::async, [this, idx]() {
        if (idx >= _size) {
            throw std::out_of_range("Index out of bounds");
        }
        auto _val = value_t {};
        auto _err = clEnqueueReadBuffer(_queue, _mem, CL_TRUE, idx * sizeof(value_t), sizeof(value_t), &_val, 0, nullptr, nullptr);
        if (_err != CL_SUCCESS) {
            throw std::runtime_error("Failed to read element");
        }
        return _val;
    });
}

template <typename value_t>
std::future<std::vector<value_t>> array_buffer<value_t>::fetch()
{
    return std::async(std::launch::async, [this]() {
        auto _result = std::vector<value_t>(_size);
        auto _err = clEnqueueReadBuffer(_queue, _mem, CL_TRUE, 0, _size * sizeof(value_t), _result.data(), 0, nullptr, nullptr);
        if (_err != CL_SUCCESS) {
            throw std::runtime_error("Failed to read array buffer");
        }
        return _result;
    });
}

template <typename value_t>
std::size_t array_buffer<value_t>::get_size() const
{
    return _size;
}

template <typename value_t>
void array_buffer<value_t>::_release()
{
    if (_mem) {
        clReleaseMemObject(_mem);
    }
}

}