#include <compute/compute.hpp>

#include <iostream>

// generated dir is in included dirs
#include "position.hpp"
#include "velocity.hpp"
#include "speed.hpp"

void print(const position& pos)
{
    std::cout << "x = " << pos.x << " y = " << pos.y << " z = " << pos.z;
    std::cout << std::endl;
}

int main()
{
    compute::device _device = compute::device::get_device(0);
    compute::context _context(_device);
    std::cout << "device name : " << _device.get_name() << std::endl;

    compute::registry _registry(_context, 1024);
    compute::entity _entity = _registry.create_entity();
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