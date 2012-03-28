#include "sobol.h"
#include "rng.h"
#include "image_sampler.h"
#include "sample.h"
#include "hilbert_curve.h"

#include <limits.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <algorithm>

#define RAND ((float)(rand_cmwc()%400000000)/(float)400000000)

inline float radical_inverse(int n, int base)
{
    float val = 0;
    float inv_base = 1.0 / base;
    float inv_bi = inv_base;
    while (n > 0) {
        int d_i = (n % base);
        val += d_i * inv_bi;
        n /= base;
        inv_bi *= inv_base;
    }
    return val;
}


ImageSampler::ImageSampler(int spp_,
                     int res_x_, int res_y_,
                     float f_width_,
                     int bucket_size_)
{
    spp = spp_;
    res_x = res_x_;
    res_y = res_y_;
    f_width = ceilf(f_width_) * 2;
    bucket_size = bucket_size_;
    
    x = 0;
    y = 0;
    s = 0;
    
    samp_taken = 0;
    tot_samp = spp * res_x * res_y;
    
    // Determine hilbert order to cover entire image
    unsigned int dim = res_x > res_y ? res_x : res_y;
    dim += f_width;
    hilbert_order = 1;
    hilbert_res = 2;
    while (hilbert_res < dim)
    {
        hilbert_res <<= 1;
        hilbert_order++;
    }
    points_traversed = 0;
    
    init_rand(17);
}                    


ImageSampler::~ImageSampler()
{
}


bool ImageSampler::get_next_sample(Sample *sample)
{
    //std::cout << s << " " << x << " " << y << std::endl;
    // Check if we're done
    if(x >= res_x+f_width || y >= res_y+f_width || points_traversed >= (hilbert_res*hilbert_res))
        return false;

    // Using sobol
    ///*
    sample->x = (sobol::sample(samp_taken, 0) + x) / res_x;
    sample->y = (sobol::sample(samp_taken, 1) + y) / res_y;
    sample->u = sobol::sample(samp_taken, 2);
    sample->v = sobol::sample(samp_taken, 3);
    sample->t = sobol::sample(samp_taken, 4);
    //*/
    
    // Using random
    /*
    sample->x = (RAND + x) / res_x;
    sample->y = (RAND + y) / res_y;
    sample->u = RAND;
    sample->v = RAND;
    sample->t = RAND;
    */
    
    // increment to next sample
    samp_taken++;
    s++;
    if(s >= spp)
    {
        s = 0;
        
        // Hilbert curve traverses pixels
        do {
            hil_inc_xy(&x, &y, hilbert_order);
            points_traversed++;
        } while(x >= res_x || y >= res_y);
    }
    
    return true;
}