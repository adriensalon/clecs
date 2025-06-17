#include <compute/core/context.hpp>

#include <stdexcept>

namespace compute {

context::context(const device& dev, const std::vector<cl_context_properties>& props)
    : _device(dev._device)
{
    auto _err = 0;
    auto _context_props = props;
    _context_props.push_back(0);
    _context = clCreateContext(_context_props.data(), 1, &_device, nullptr, nullptr, &_err);
    if (_err != CL_SUCCESS || !_context) {
        throw std::runtime_error("Failed to create OpenCL context.");
    }
    _queue = clCreateCommandQueue(_context, _device, 0, &_err);
    if (_err != CL_SUCCESS || !_queue) {
        clReleaseContext(_context);
        throw std::runtime_error("Failed to create OpenCL command queue.");
    }
}

context::~context()
{
    if (_queue) {
        clReleaseCommandQueue(_queue);
    }
    if (_context) {
        clReleaseContext(_context);
    }
}

context::context(context&& other) noexcept
    : _device(other._device)
    , _context(other._context)
    , _queue(other._queue)
{
    other._context = nullptr;
    other._queue = nullptr;
}

context& context::operator=(context&& other) noexcept
{
    if (this != &other) {
        if (_queue) {
            clReleaseCommandQueue(_queue);
        }
        if (_context) {
            clReleaseContext(_context);
        }
        _device = other._device;
        _context = other._context;
        _queue = other._queue;
        other._context = nullptr;
        other._queue = nullptr;
    }
    return *this;
}

}
