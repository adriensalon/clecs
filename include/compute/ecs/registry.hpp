#pragma once

#include <compute/core/buffer.hpp>
#include <compute/core/context.hpp>
#include <compute/core/kernel.hpp>
#include <compute/ecs/entity.hpp>

#include <typeindex>

namespace compute {

/// @brief Central coordinator for device-resident ECS data and system execution.
/// The `registry` manages creation of entities, association of component data
/// (stored on device), and execution of systems via OpenCL kernels. All components
/// are stored in contiguous device memory using `array_buffer<component_t>`, allowing
/// compute kernels to process large sets of entities in parallel.
/// It is the main interface users interact with to construct ECS scenes,
/// assign components, and run systems on the device.
/// @note The registry does not store component values on the host; it assumes
/// all components reside and are processed in device memory.
struct registry {

    registry(const registry& other) = delete;
    registry& operator=(const registry& other) = delete;
    registry(registry&& other) noexcept = default;
    registry& operator=(registry&& other) noexcept = default;

    /// @brief Constructs a new ECS registry backed by a given GPU context.
    /// @param ctx The device context used for all memory allocations and kernel launches.
    /// @param capacity Pre-allocated number of entities/components (default: 1024).
    registry(const context& ctx, size_t capacity = 1024);

    /// @brief Creates a new entity.
    /// Returns a unique `entity` identifier. The entity initially has no components.
    /// Component storage for entities is managed by the registry internally.
    /// @return A unique entity handle.
    [[nodiscard]] entity create_entity();

    /// @brief Adds a component to the given entity.
    /// If the component type has not yet been registered, a device buffer for that
    /// component type is automatically allocated. The value is copied to the device.
    /// @tparam component_t The type of component being added.
    /// @param e The target entity.
    /// @param value The value to assign to this entity’s component.
    template <typename component_t>
    void add_component(entity e, const component_t& value);

    /// @brief Asynchronously retrieves a component's value from the device.
    /// This performs a non-blocking read of the component associated with an entity,
    /// returning a `std::future` that resolves with the host-side copy.
    /// @tparam component_t The component type to fetch.
    /// @param e The entity whose component should be fetched.
    /// @return A future resolving to the component value.
    template <typename component_t>
    [[nodiscard]] std::future<component_t> get_component(entity e);

    /// @brief Executes a user-defined system over the specified component types.
    /// This method prepares the component buffers as kernel arguments and
    /// dispatches a compute kernel generated from the user-defined `system_t`.
    /// The kernel will be launched with global work size equal to entity capacity.
    template <typename system_t, typename... components_t>
    void execute_system();

    // serialize to & from binary std::string with cereal ?

private:
    const context& _context;
    std::size_t _capacity;
    std::uint32_t _next_entity;
    std::unordered_map<std::type_index, std::shared_ptr<void>> _component_stores;
    std::unordered_map<std::type_index, std::unordered_map<entity, std::size_t>> _entity_component_map;
    template <typename component_t>
    std::shared_ptr<compute::array_buffer<component_t>> _get_or_create_component_store();
};

}

#include "registry.inl"
