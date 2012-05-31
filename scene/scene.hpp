/*
 * This file and scene.cpp define a Scene class, which is used to build and
 * store a scene description to be rendered.
 */
#ifndef SCENE_H
#define SCENE_H

#include "numtype.h"

#include <vector>

#include "camera.hpp"
#include "bvh.hpp"
#include "primitive.hpp"

/*
 * The Scene class is used to build and store the complete description of a 3d
 * scene to be rendered.
 *
 * The scene is built via the various methods to add primitives, lights, and
 * shaders.  It must be finalized (which initializes important acceleration
 * structures, etc.) before being passed off for rendering.
 */
struct Scene {
	Camera *camera;
	std::vector<Primitive *> primitives;
	BVH world;


	void add_primitive(Primitive *primitive) {
		primitives.push_back(primitive);
	}

	// Finalizes the scene for rendering
	void finalize() {
		world.add_primitives(primitives);
		world.finalize();
	}

	bool intersect_ray(Ray &ray, Intersection *intersection=NULL) {
		return world.intersect_ray(ray, intersection);
	}
};

#endif // SCENE_H