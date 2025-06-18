#pragma once

#include <cstdint>

namespace compute {

/// @brief Unique identifier for an entity in the ECS.
/// Entities are represented as 32-bit unsigned integers. They act as handles
/// that associate component data stored in device memory via ECS buffers.
using entity = std::uint32_t;

}
