#pragma once

#include <compute/core/buffer.hpp>
#include <compute/core/context.hpp>
#include <compute/core/kernel.hpp>
#include <compute/ecs/entity.hpp>

namespace compute {

/// @brief
struct registry {

    registry(const registry& other) = delete;
    registry& operator=(const registry& other) = delete;
    registry(registry&& other) noexcept = default;
    registry& operator=(registry&& other) noexcept = default;

    /// @brief 
    /// @param ctx 
    /// @param capacity 
    registry(const context& ctx, size_t capacity = 1024);

    /// @brief 
    /// @return 
    entity create_entity();

    /// @brief 
    /// @tparam component_t 
    /// @param e 
    /// @param value 
    template <typename component_t>
    void add_component(entity e, const component_t& value);

    /// @brief 
    /// @tparam component_t 
    /// @param e 
    /// @return 
    template <typename component_t>
    [[nodiscard]] std::future<component_t> get_component(entity e);

    /// @brief 
    /// @tparam system_t 
    /// @tparam ...components_t 
    template <typename system_t, typename... components_t>
    void execute_system();

private:
    const context& _context;
    std::size_t _capacity;
    std::uint32_t _next_entity;
    std::unordered_map<std::type_index, std::shared_ptr<void>> _component_stores;
    std::unordered_map<std::type_index, std::unordered_map<entity, size_t>> _entity_component_map;

    template <typename component_t>
    std::shared_ptr<compute::array_buffer<component_t>> _get_or_create_component_store();
};

}

#include "registry.inl"
