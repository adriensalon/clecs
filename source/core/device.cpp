#pragma once

#include <compute/core/device.hpp>

#include <stdexcept>

namespace compute {

device::device(const cl_platform_id plat, const cl_device_id dev)
{
    _platform = plat;
    _device = dev;
}

device::device(device&& other) noexcept
    : _platform(other._platform)
    , _device(other._device)
{
    other._platform = nullptr;
    other._device = nullptr;
}

device& device::operator=(device&& other) noexcept
{
    if (this != &other) {
        _platform = other._platform;
        _device = other._device;
        other._platform = nullptr;
        other._device = nullptr;
    }
    return *this;
}

std::size_t device::get_devices_count()
{
    cl_uint num_platforms = 0;
    cl_int err = clGetPlatformIDs(0, nullptr, &num_platforms);
    if (err != CL_SUCCESS || num_platforms == 0) {
        throw std::runtime_error("Failed to find any OpenCL platforms.");
    }
    return static_cast<std::size_t>(num_platforms);
}

std::vector<device> device::get_all_devices()
{
    cl_uint num_platforms = 0;
    cl_int err = clGetPlatformIDs(0, nullptr, &num_platforms);
    if (err != CL_SUCCESS || num_platforms == 0) {
        throw std::runtime_error("Failed to find any OpenCL platforms.");
    }

    std::vector<cl_platform_id> platforms(num_platforms);
    err = clGetPlatformIDs(num_platforms, platforms.data(), nullptr);
    if (err != CL_SUCCESS) {
        throw std::runtime_error("Failed to get OpenCL platform IDs.");
    }

    std::vector<device> devices;

    for (const auto& platform : platforms) {
        cl_uint num_devices = 0;
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, nullptr, &num_devices);
        if (err != CL_SUCCESS || num_devices == 0) {
            continue; // Skip platforms with no available devices
        }

        std::vector<cl_device_id> devs(num_devices);
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, num_devices, devs.data(), nullptr);
        if (err != CL_SUCCESS) {
            continue;
        }

        for (const auto& dev : devs) {
            devices.emplace_back(platform, dev);
        }
    }

    return devices;
}

device device::get_device(const std::size_t idx)
{
    if (idx >= get_devices_count()) {
        throw std::out_of_range("Device index out of range");
    }
    return std::move(get_all_devices()[idx]);
}

std::string device::get_name() const
{
    if (!_device) {
        throw std::runtime_error("Invalid OpenCL device.");
    }

    size_t size = 0;
    cl_int err = clGetDeviceInfo(_device, CL_DEVICE_NAME, 0, nullptr, &size);
    if (err != CL_SUCCESS) {
        throw std::runtime_error("Failed to get OpenCL device name size.");
    }

    std::vector<char> name(size);
    err = clGetDeviceInfo(_device, CL_DEVICE_NAME, size, name.data(), nullptr);
    if (err != CL_SUCCESS) {
        throw std::runtime_error("Failed to get OpenCL device name.");
    }

    return std::string(name.data());
}

}
