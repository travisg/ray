#ifndef __SCENE_H
#define __SCENE_H

#include <vector>

class Sphere;
class Ray;

class Scene {
public:
	Scene();
	virtual ~Scene();

	const Sphere *Intersect(const Ray &ray);
	bool DoesIntersect(const Ray &ray);
	
private:
	std::vector<Sphere *> slist;

};

#endif

