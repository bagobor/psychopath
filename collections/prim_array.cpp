#include "numtype.h"

#include <iostream>
#include "ray.hpp"
#include "prim_array.hpp"



PrimArray::~PrimArray()
{
	int32_t size = children.size();

	for (int32_t i=0; i < size; i++) {
		delete children[i];
	}
}

void PrimArray::add_primitives(std::vector<Primitive *> &primitives)
{
	int32_t start = children.size();
	int32_t added = primitives.size();
	children.resize(start + added);

	for (int32_t i=0; i < added; i++) {
		children[start + i] = primitives[i];
	}
}

bool PrimArray::finalize()
{
	// Initialize primitive's bounding boxes
	uint32_t s = children.size();
	for (uint32_t i = 0; i < s; i++) {
		children[i]->bounds();
	}

	return true;
}

size_t PrimArray::max_primitive_id() const
{
	return children.size();
}

Primitive &PrimArray::get_primitive(size_t id)
{
	return *(children[id]);
}

BBoxT &PrimArray::bounds()
{
	return bbox;
}

bool PrimArray::intersect_ray(const Ray &ray, Intersection *intersection)
{
	std::vector<Primitive *> temp_prim;
	float tnear, tfar;
	bool hit = false;
	int32_t size = children.size();

	for (int32_t i=0; i < size; i++) {
		if (children[i]->bounds().intersect_ray(ray, &tnear, &tfar)) {
			// Trace!
			hit |= children[i]->intersect_ray(ray, intersection);

			// Early out for shadow rays
			if (hit && ray.is_shadow_ray)
				break;
		}
	}

	return hit;
}


uint PrimArray::get_potential_intersections(const Ray &ray, float tmax, uint max_potential, size_t *ids, void *state)
{
	const uint32_t size = children.size();
	float tnear, tfar;

	// Fetch starting index
	uint32_t i = 0;
	if (state != nullptr)
		i = *((uint64_t *)state);

	static uint count = 0;
	count++;
	if (!(count%20000))
		std::cout << "State: " << i << std::endl;

	// Accumulate potential primitive intersections
	uint32_t hits_so_far = 0;
	for (; i < size && hits_so_far < max_potential; i++) {
		if (children[i]->bounds().intersect_ray(ray, &tnear, &tfar) && tnear < tmax) {
			ids[hits_so_far] = i;
			hits_so_far++;
		}
	}

	// Write last index
	if (state != nullptr)
		*((uint64_t *)state) = i;

	return hits_so_far;
}
