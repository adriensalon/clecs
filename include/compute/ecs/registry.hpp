#pragma once

#include <compute/core/buffer.hpp>
#include <compute/core/context.hpp>
#include <compute/core/kernel.hpp>
#include <compute/ecs/entity.hpp>

namespace compute {

/// @brief
struct registry {
    registry(const context& ctx, size_t capacity = 1024)
        : _context(ctx)
        , _capacity(capacity)
        , _next_entity(0)
    {
    }

    entity create_entity()
    {
        return entity { _next_entity++ };
    }

    template <typename component_t>
    void add_shared_components(const shared_buffer& buf, const std::vector<entity>& es = {});

    template <typename component_t>
    void add_component(entity e, const component_t& value)
    {
        auto& entity_map = _entity_component_map[std::type_index(typeid(component_t))];

        if (entity_map.find(e) != entity_map.end()) {
            throw std::runtime_error("Component already added to entity");
        }

        auto buffer = get_or_create_component_store<component_t>();
        size_t idx = entity_map.size(); // Current count of components of this type

        if (idx >= buffer->get_size()) {
            throw std::runtime_error("Exceeded component buffer capacity");
        }

        // Store index for this entity
        entity_map[e] = idx;

        // Upload value to OpenCL buffer
        std::vector<component_t> temp = buffer->fetch().get();
        temp[idx] = value;
        buffer->set(temp);
    }

    template <typename component_t>
    [[nodiscard]] std::future<component_t> get_component(entity e)
    {
        const auto& entity_map = _entity_component_map.at(std::type_index(typeid(component_t)));

        if (entity_map.find(e) == entity_map.end())
            throw std::runtime_error("Component not found for entity");

        size_t idx = entity_map.at(e);
        auto buffer = std::static_pointer_cast<compute::array_buffer<component_t>>(
            _component_stores.at(std::type_index(typeid(component_t))));

        return std::async(std::launch::async, [buffer, idx]() {
            std::vector<component_t> all = buffer->fetch().get();
            return all.at(idx);
        });
    }

    template <typename system_t, typename... components_t>
    void execute_system() // system data?
    {
        compute::kernel krn(_context, system_t::kernel_source, "system_main");
        size_t entity_count = _next_entity;
        int idx = 0;
        (krn.set_arg(idx++, *get_or_create_component_store<components_t>()), ...);
        krn.run({ entity_count });
    }

private:
    const context& _context;
    std::size_t _capacity;
    std::uint32_t _next_entity;

    // Component storage: maps component type -> array_buffer<component_t>
    std::unordered_map<std::type_index, std::shared_ptr<void>> _component_stores;

    // Entity to index mapping per component type
    std::unordered_map<std::type_index, std::unordered_map<entity, size_t>> _entity_component_map;

    template <typename component_t>
    std::shared_ptr<compute::array_buffer<component_t>> get_or_create_component_store()
    {
        auto type = std::type_index(typeid(component_t));

        if (_component_stores.find(type) != _component_stores.end()) {
            return std::static_pointer_cast<compute::array_buffer<component_t>>(_component_stores[type]);
        }

        auto buffer = std::make_shared<compute::array_buffer<component_t>>(_context, _capacity);
        _component_stores[type] = buffer;
        return buffer;
    }
};

}
