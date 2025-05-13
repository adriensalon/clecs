#pragma once

#include <compute/core/opencl.hpp>

#include <vector>
#include <string>

namespace compute {

/// @brief
struct device {

    device(const device& other) = delete;
    device& operator=(const device& other) = delete;
    device(device&& other) noexcept;
    device& operator=(device&& other) noexcept;

    /// @brief
    /// @param platform
    /// @param device
    device(const cl_platform_id plat, const cl_device_id dev);

    [[nodiscard]] std::string get_name() const;

    /// @brief
    /// @return
    [[nodiscard]] static std::size_t get_devices_count();

    /// @brief
    /// @param idx
    /// @return
    [[nodiscard]] static device get_device(const std::size_t idx);

    /// @brief
    /// @return
    [[nodiscard]] static std::vector<device> get_all_devices();

private:
    cl_platform_id _platform;
    cl_device_id _device;
    friend struct context;
};

}
