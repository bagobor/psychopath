#include "path_trace_integrator.hpp"

#include <iostream>
#include <assert.h>
#include "image_sampler.hpp"
#include "film.hpp"
#include "intersection.hpp"
#include "tracer.hpp"
#include "config.hpp"

#include "array.hpp"

#include "light.hpp"

#include "hilbert.hpp"

#define RAYS_AT_A_TIME 1000000


float32 lambert(Vec3 v1, Vec3 v2)
{
	v1.normalize();
	v2.normalize();
	float32 f = dot(v1, v2);
	if (f < 0.0)
		f = 0.0;
	return f;
}


void PathTraceIntegrator::integrate()
{
	RNG rng(43643);
	ImageSampler image_sampler(spp, image->width_p, image->height_p);

	// Sample array
	Array<Sample> samps;
	samps.resize(RAYS_AT_A_TIME);

	// Light path array
	Array<PTPath> paths;
	paths.resize(RAYS_AT_A_TIME);

	// Allocate the RayInter structures in contiguous storage
	RayInter *rayinters_ = new RayInter[RAYS_AT_A_TIME];

	// RayInter pointer array, points to contiguous storage above
	Array<RayInter *> rayinters;
	rayinters.reserve(RAYS_AT_A_TIME);
	for (uint32 i=0; i < RAYS_AT_A_TIME; i++) {
		rayinters.push_back(&rayinters_[i]);
	}

	bool last = false;
	while (true) {
		// Generate samples
		std::cout << "\t--------\n\tGenerating samples" << std::endl;
		std::cout.flush();
		for (int i = 0; i < RAYS_AT_A_TIME; i++) {
			paths[i].done = false;
			paths[i].col = Color(0.0);
			paths[i].fcol = Color(1.0);

			if (!image_sampler.get_next_sample(&(samps[i]), path_length*5)) {
				samps.resize(i);
				paths.resize(i);
				last = true;
				break;
			}
		}
		uint32 samp_size = samps.size();

		// Path tracing loop for the samples we have
		for (int path_n=0; path_n < path_length; path_n++) {
			std::cout << "\t-- Path segment #" << path_n << std::endl;
			std::cout.flush();

			// Size the ray buffer appropriately
			rayinters.resize(samp_size);

			int32 so = (path_n * 5) - 2; // Sample offset

			// Calculate path rays
			std::cout << "\tGenerating path rays" << std::endl;
			std::cout.flush();
			if (path_n == 0) {
				// First segment of path is camera rays
				for (uint32 i = 0; i < samp_size; i++) {
					float32 rx = (samps[i].x - 0.5) * (image->max_x_p - image->min_x_p);
					float32 ry = (0.5 - samps[i].y) * (image->max_y_p - image->min_y_p);
					float32 dx = (image->max_x_p - image->min_x_p) / image->width_p;
					float32 dy = (image->max_y_p - image->min_y_p) / image->height_p;
					rayinters[i]->ray = scene->camera->generate_ray(rx, ry, dx, dy, samps[i].t, samps[i].u, samps[i].v);
					rayinters[i]->ray.finalize();
					rayinters[i]->hit = false;
					rayinters[i]->id = i;
				}
			} else {
				// Other path segments are bounces
				uint32 pri = 0; // Path ray index
				for (uint32 i = 0; i < samp_size; i++) {
					if (!paths[i].done) {
						// Generate a random ray direction in the hemisphere
						// of the surface.
						// TODO: use BxDF distribution here
						// TODO: use proper PDF here
						Vec3 dir = cosine_sample_hemisphere(samps[i].ns[so], samps[i].ns[so+1]);
						float32 pdf = dir.z * 2;
						//Vec3 dir = uniform_sample_hemisphere(samps[i].ns[so], samps[i].ns[so+1]);
						//float32 pdf = 1.0;

						if (pdf < 0.001)
							pdf = 0.001;
						dir = zup_to_vec(dir, paths[i].inter.n);

						// Calculate the color filtering effect that the
						// bounce from the current intersection will create.
						// TODO: use actual shaders here.
						paths[i].fcol *= lambert(dir, paths[i].inter.n) / pdf;

						// Clear out the rayinter structure
						rayinters[pri]->hit = false;
						rayinters[pri]->id = i;

						// Create a bounce ray for this path
						rayinters[pri]->ray.o = paths[i].inter.p;
						rayinters[pri]->ray.d = dir;
						rayinters[pri]->ray.time = samps[i].t;
						rayinters[pri]->ray.is_shadow_ray = false;
						rayinters[pri]->ray.has_differentials = false;
						rayinters[pri]->ray.min_t = 0.01;
						rayinters[pri]->ray.max_t = 999999999999.0;
						rayinters[pri]->ray.finalize();

						// Increment path ray index
						pri++;
					}
				}
				rayinters.resize(pri);
			}


			// Trace the rays
			tracer->queue_rays(rayinters);
			tracer->trace_rays();


			// Update paths
			std::cout << "\tUpdating paths" << std::endl;
			std::cout.flush();
			uint32 rsize = rayinters.size();
			for (uint32 i = 0; i < rsize; i++) {
				const uint32 id = rayinters[i]->id;
				if (rayinters[i]->hit) {
					// Ray hit something!  Store intersection data
					paths[id].inter = rayinters[i]->inter;
				} else {
					// Ray didn't hit anything, done and black background
					paths[id].done = true;
					paths[id].col += Color(0.0);
				}
			}


			// Generate a bunch of shadow rays
			std::cout << "\tGenerating shadow rays" << std::endl;
			std::cout.flush();
			uint32 sri = 0; // Shadow ray index
			for (uint32 i = 0; i < paths.size(); i++) {
				if (!paths[i].done) {
					// Select a light and store the normalization factor for it's output
					Light *lighty = scene->finite_lights[(uint32)(samps[i].ns[so+2] * scene->finite_lights.size()) % scene->finite_lights.size()];
					//Light *lighty = scene->finite_lights[rng.next_uint() % scene->finite_lights.size()];

					// Sample the light source
					Vec3 ld;
					paths[i].lcol = lighty->sample(paths[i].inter.p, samps[i].ns[so+3], samps[i].ns[so+4], samps[i].t, &ld)
					                * (float32)(scene->finite_lights.size());
					//paths[i].lcol = lighty->sample(paths[i].inter.p, rng.next_float(), rng.next_float(), samps[i].t, &ld)
					//                * (float32)(scene->finite_lights.size());

					// Create a shadow ray for this path
					float d = ld.length();
					ld.normalize();
					rayinters[sri]->ray.o = paths[i].inter.p;
					rayinters[sri]->ray.d = ld;
					rayinters[sri]->ray.time = samps[i].t;
					rayinters[sri]->ray.is_shadow_ray = true;
					rayinters[sri]->ray.has_differentials = false;
					rayinters[sri]->ray.min_t = 0.01;
					rayinters[sri]->ray.max_t = d;
					rayinters[sri]->ray.finalize();
					rayinters[sri]->hit = false;
					rayinters[sri]->id = i;
					sri++;
				}
			}
			rayinters.resize(sri);


			// Trace the shadow rays
			tracer->queue_rays(rayinters);
			tracer->trace_rays();


			// Calculate sample colors
			std::cout << "\tCalculating sample colors" << std::endl;
			std::cout.flush();
			rsize = rayinters.size();
			for (uint32 i = 0; i < rsize; i++) {
				const uint32 id = rayinters[i]->id;
				if (!rayinters[i]->hit) {
					// Sample was lit
					// TODO: use actual shaders here
					float lam = lambert(rayinters[i]->ray.d, paths[id].inter.n);
					paths[id].col += paths[id].fcol * paths[id].lcol * lam;
				}
			}
		}

		// Accumulate the samples
		std::cout << "\tAccumulating samples" << std::endl;
		std::cout.flush();
		for (uint32 i = 0; i < samp_size; i++) {
			image->add_sample(paths[i].col, samps[i].x, samps[i].y);
		}

		// Print percentage complete
		float perc = image_sampler.percentage() * 100;
		uint32 pr = std::cout.precision();
		std::cout.precision(4);
		std::cout << perc << "%" << std::endl;
		std::cout.precision(pr);

		// Callback
		if (callback)
			(*callback)();

		if (last)
			break;
	}

	// Delete the RayInter structures
	delete [] rayinters_;

	std::cout << "Splits during rendering: " << Config::split_count << std::endl;
	std::cout << "Micropolygons generated during rendering: " << Config::upoly_gen_count << std::endl;
	std::cout << "Grid cache misses during rendering: " << Config::cache_misses << std::endl;
}
