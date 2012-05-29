#ifndef CONFIG_H
#define CONFIG_H

#include "numtype.h"

namespace Config
{
extern float32 dice_rate;
extern float32 focus_factor;
extern float32 min_upoly_size;
extern int max_grid_size;
extern int grid_cache_size;

// Statistics
extern uint64 split_count;
extern uint64 upoly_gen_count;
extern uint64 cache_misses;
}

#endif
