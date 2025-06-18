#pragma once

#include <compute/core/opencl.hpp>

#include <vector>
#include <string>

namespace compute {

/// @brief Represents an OpenCL-capable compute device.
/// This class abstracts a specific OpenCL device and its associated platform.
/// It is a lightweight, movable handle used to query device information or create
/// contexts for device-based computation. Devices cannot be copied, but they can be 
/// moved.
struct device {

    device(const device& other) = delete;
    device& operator=(const device& other) = delete;
    device(device&& other) noexcept;
    device& operator=(device&& other) noexcept;

    /// @brief Constructs a device from an OpenCL platform and device ID.
    /// @param plat The OpenCL platform associated with the device.
    /// @param dev The OpenCL device ID.
    device(const cl_platform_id plat, const cl_device_id dev);

    /// @brief Retrieves the name of the device (e.g., device model or vendor string).
    /// @return A human-readable string representing the device name.
    [[nodiscard]] std::string get_name() const;

    /// @brief Returns the total number of OpenCL-compatible devices available on the system.
    /// @return Number of available OpenCL devices.
    [[nodiscard]] static std::size_t get_devices_count();

    /// @brief Retrieves a device by index. This can be used for selecting a specific 
    /// compute device among those available.
    /// @param idx Index of the desired device [0..get_devices_count()).
    /// @return A `device` instance corresponding to the given index.
    [[nodiscard]] static device get_device(const std::size_t idx);

    /// @brief Enumerates all available OpenCL-compatible compute devices.
    /// @return A vector containing all detected compute devices.
    [[nodiscard]] static std::vector<device> get_all_devices();

private:
    cl_platform_id _platform;
    cl_device_id _device;
    friend struct context;
};

}
