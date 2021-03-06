#include "numtype.h"

#include "config.hpp"

namespace Config
{
float dice_rate = 0.5; // 0.7 is about half pixel area
float min_upoly_size = 0.001; // Approximate minimum micropolygon size in world space
size_t max_grid_size = 64;
float grid_cache_size = 128.0; // In MB

float displace_distance = 0.00f;
}
