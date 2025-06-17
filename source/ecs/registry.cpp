#include <compute/ecs/registry.hpp>

namespace compute {

registry::registry(const context& ctx, size_t capacity)
    : _context(ctx)
    , _capacity(capacity)
    , _next_entity(0)
{
}

entity registry::create_entity()
{
    return entity { _next_entity++ };
}

}