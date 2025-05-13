#include <compute/core/kernel.hpp>

#include <stdexcept>

namespace compute {

kernel::kernel(const context& ctx, const std::string& code, const std::string& name)
    : _device(ctx._device)
    , _context(ctx._context)
    , _command_queue(ctx._queue)
{
    cl_int err;

    const char* source = code.c_str();
    size_t length = code.length();

    _program = clCreateProgramWithSource(_context, 1, &source, &length, &err);
    if (err != CL_SUCCESS) {
        throw std::runtime_error("Failed to create OpenCL program.");
    }

    err = clBuildProgram(_program, 1, &_device, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS) {
        // Optional: Get build log
        size_t log_size = 0;
        clGetProgramBuildInfo(_program, _device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size);
        std::string build_log(log_size, '\0');
        clGetProgramBuildInfo(_program, _device, CL_PROGRAM_BUILD_LOG, log_size, &build_log[0], nullptr);

        clReleaseProgram(_program);
        throw std::runtime_error("Failed to build OpenCL program:\n" + build_log);
    }

    _kernel = clCreateKernel(_program, name.c_str(), &err);
    if (err != CL_SUCCESS) {
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
        if (_kernel)
            clReleaseKernel(_kernel);
        if (_program)
            clReleaseProgram(_program);
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
    if (_kernel)
        clReleaseKernel(_kernel);
    if (_program)
        clReleaseProgram(_program);
}

void kernel::run(const std::vector<std::size_t>& wsz)
{
    std::vector<size_t> global_ws = wsz;
    if (global_ws.empty()) {
        throw std::runtime_error("Work size cannot be empty.");
    }
    cl_int err = clEnqueueNDRangeKernel(
        _command_queue,
        _kernel,
        static_cast<cl_uint>(global_ws.size()),
        nullptr,
        global_ws.data(),
        nullptr, // No local work size
        0, nullptr, nullptr);

    if (err != CL_SUCCESS) {
        throw std::runtime_error("Failed to enqueue kernel.");
    }
    clFinish(_command_queue);
}
}
