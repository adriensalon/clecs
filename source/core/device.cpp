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
    auto _num_platforms = 0u;
    auto _err = clGetPlatformIDs(0, nullptr, &_num_platforms);
    if (_err != CL_SUCCESS || _num_platforms == 0) {
        throw std::runtime_error("Failed to find any OpenCL platforms.");
    }
    return static_cast<std::size_t>(_num_platforms);
}

std::vector<device> device::get_all_devices()
{
    auto _num_platforms = 0u;
    auto _err = clGetPlatformIDs(0, nullptr, &_num_platforms);
    if (_err != CL_SUCCESS || _num_platforms == 0) {
        throw std::runtime_error("Failed to find any OpenCL platforms.");
    }
    auto _platforms = std::vector<cl_platform_id>(_num_platforms);
    _err = clGetPlatformIDs(_num_platforms, _platforms.data(), nullptr);
    if (_err != CL_SUCCESS) {
        throw std::runtime_error("Failed to get OpenCL platform IDs.");
    }
    auto devices = std::vector<device> {};
    for (const auto& platform : _platforms) {
        auto _num_devices = 0u;
        _err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, nullptr, &_num_devices);
        if (_err != CL_SUCCESS || _num_devices == 0) {
            continue;
        }
        auto _devs = std::vector<cl_device_id>(_num_devices);
        _err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, _num_devices, _devs.data(), nullptr);
        if (_err != CL_SUCCESS) {
            continue;
        }
        for (const auto& dev : _devs) {
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
    auto _size = static_cast<std::size_t>(0u);
    auto _err = clGetDeviceInfo(_device, CL_DEVICE_NAME, 0, nullptr, &_size);
    if (_err != CL_SUCCESS) {
        throw std::runtime_error("Failed to get OpenCL device name size.");
    }
    auto _name = std::vector<char>(_size);
    _err = clGetDeviceInfo(_device, CL_DEVICE_NAME, _size, _name.data(), nullptr);
    if (_err != CL_SUCCESS) {
        throw std::runtime_error("Failed to get OpenCL device name.");
    }
    return std::string(_name.data());
}

}
