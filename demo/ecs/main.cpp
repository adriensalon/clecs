#include <compute/ecs/entity.hpp>
#include <compute/ecs/registry.hpp>

#include <iostream>

// generated dir is in included dirs
#include "position.hpp"
#include "speed.hpp"

void print(const position& pos)
{
    std::cout << "x = " << pos.x << " y = " << pos.y << " z = " << pos.z;
    std::cout << std::endl;
}

int main()
{
    auto _device = compute::device::get_device(0);
    auto _context = compute::context(_device);
    std::cout << "device name : " << _device.get_name() << std::endl;

    auto _registry = compute::registry(_context, 1024);
    auto _entity = _registry.create_entity();
    _registry.add_component<position>(_entity, position{});

    print(_registry.get_component<position>(_entity).get());
    _registry.execute_system<speed, position>();
    print(_registry.get_component<position>(_entity).get());
    _registry.execute_system<speed, position>();
    print(_registry.get_component<position>(_entity).get());
    _registry.execute_system<speed, position>();
    print(_registry.get_component<position>(_entity).get());
    _registry.execute_system<speed, position>();
    print(_registry.get_component<position>(_entity).get());


    return 0;
}