#include <compute/core/kernel.hpp>

#include <stdexcept>

namespace compute {

kernel::kernel(const context& ctx, const std::string& code, const std::string& name)
    : _device(ctx._device)
    , _context(ctx._context)
    , _command_queue(ctx._queue)
{
    auto _err = 0;
    auto* _source = code.c_str();
    auto _length = code.length();
    _program = clCreateProgramWithSource(_context, 1, &_source, &_length, &_err);
    if (_err != CL_SUCCESS) {
        throw std::runtime_error("Failed to create OpenCL program.");
    }
    _err = clBuildProgram(_program, 1, &_device, nullptr, nullptr, nullptr);
    if (_err != CL_SUCCESS) {
        auto _log_size = static_cast<std::size_t>(0);
        clGetProgramBuildInfo(_program, _device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &_log_size);
        auto _build_log = std::string(_log_size, '\0');
        clGetProgramBuildInfo(_program, _device, CL_PROGRAM_BUILD_LOG, _log_size, &_build_log[0], nullptr);
        clReleaseProgram(_program);
        throw std::runtime_error("Failed to build OpenCL program:\n" + _build_log);
    }
    _kernel = clCreateKernel(_program, name.c_str(), &_err);
    if (_err != CL_SUCCESS) {
        clReleaseProgram(_program);
        throw std::runtime_error("Failed to create OpenCL kernel.");
    }
}

kernel::kernel(kernel&& other) noexcept
    : _device(other._device)
    , _context(other._context)
    , _command_queue(other._command_queue)
    , _program(other._program)
    , _kernel(other._kernel)
{
    other._program = nullptr;
    other._kernel = nullptr;
    other._device = nullptr;
    other._context = nullptr;
    other._command_queue = nullptr;
}

kernel& kernel::operator=(kernel&& other) noexcept
{
    if (this != &other) {
        if (_kernel) {
            clReleaseKernel(_kernel);
        }
        if (_program) {
            clReleaseProgram(_program);
        }
        _device = other._device;
        _context = other._context;
        _command_queue = other._command_queue;
        _program = other._program;
        _kernel = other._kernel;
        other._device = nullptr;
        other._context = nullptr;
        other._command_queue = nullptr;
        other._program = nullptr;
        other._kernel = nullptr;
    }
    return *this;
}

kernel::~kernel()
{
    if (_kernel) {
        clReleaseKernel(_kernel);
    }
    if (_program) {
        clReleaseProgram(_program);
    }
}

std::future<void> kernel::run(const std::vector<std::size_t>& wsz)
{
    return std::async(std::launch::async, [this, wsz]() {
        auto _global_ws = wsz;
        if (_global_ws.empty()) {
            throw std::runtime_error("Work size cannot be empty.");
        }
        auto _err = clEnqueueNDRangeKernel(_command_queue, _kernel, static_cast<cl_uint>(_global_ws.size()), nullptr, _global_ws.data(), nullptr, 0, nullptr, nullptr);
        if (_err != CL_SUCCESS) {
            throw std::runtime_error("Failed to enqueue kernel.");
        }
        clFinish(_command_queue);
    });
}

}
