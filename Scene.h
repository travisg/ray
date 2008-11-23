#ifndef __SCENE_H
#define __SCENE_H

#include <vector>

class Drawable;
class Ray;

class Scene {
public:
	Scene();
	virtual ~Scene();

	const Drawable *Intersect(const Ray &ray);
	bool DoesIntersect(const Ray &ray);
	
private:
	std::vector<Drawable *> slist;
};

#endif

