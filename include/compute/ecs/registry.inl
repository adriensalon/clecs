namespace compute {

template <typename component_t>
void registry::add_component(entity e, const component_t& value)
{
    auto& _entity_map = _entity_component_map[std::type_index(typeid(component_t))];
    if (_entity_map.find(e) != _entity_map.end()) {
        throw std::runtime_error("Component already added to entity");
    }
    auto _buffer = _get_or_create_component_store<component_t>();
    auto _idx = _entity_map.size();
    if (_idx >= _buffer->get_size()) {
        throw std::runtime_error("Exceeded component buffer capacity");
    }
    _entity_map[e] = _idx;
    auto _temp = _buffer->fetch().get();
    _temp[_idx] = value;
    _buffer->set(_temp);
}

template <typename component_t>
std::future<component_t> registry::get_component(entity e)
{
    const auto& _entity_map = _entity_component_map.at(std::type_index(typeid(component_t)));
    if (_entity_map.find(e) == _entity_map.end()) {
        throw std::runtime_error("Component not found for entity");
    }
    auto _idx = _entity_map.at(e);
    auto _buffer = std::static_pointer_cast<compute::array_buffer<component_t>>(_component_stores.at(std::type_index(typeid(component_t))));
    return std::async(std::launch::async, [_buffer, _idx]() {
        std::vector<component_t> all = _buffer->fetch().get();
        return all.at(_idx);
    });
}

template <typename system_t, typename... components_t>
void registry::execute_system()
{
    auto _krn = compute::kernel(_context, system_t::kernel_source, "system_main");
    auto _entity_count = _next_entity;
    auto _idx = 0;
    (_krn.set_arg(_idx++, *_get_or_create_component_store<components_t>()), ...);
    _krn.run({ _entity_count });
}

template <typename component_t>
std::shared_ptr<compute::array_buffer<component_t>> registry::_get_or_create_component_store()
{
    auto _type = std::type_index(typeid(component_t));
    if (_component_stores.find(_type) != _component_stores.end()) {
        return std::static_pointer_cast<compute::array_buffer<component_t>>(_component_stores[_type]);
    }
    auto _buffer = std::make_shared<compute::array_buffer<component_t>>(_context, _capacity);
    _component_stores[_type] = _buffer;
    return _buffer;
}

}