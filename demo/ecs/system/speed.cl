#include "position.cl"

kernel void smain(__global position* positions)
{
    int k = get_global_id(0);
    positions[k].x = 1.2f + positions[k].x;
    positions[k].y = 1.4f + positions[k].y;
    positions[k].z = 1.6f + positions[k].z;
} 
