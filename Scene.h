#ifndef __SCENE_H
#define __SCENE_H

#include <vector>
#include "Color.h"

class Drawable;
class Ray;
class SimpleLight;

class Scene {
public:
	Scene();
	virtual ~Scene();

	const Drawable *Intersect(const Ray &ray);
	bool DoesIntersect(const Ray &ray);
	bool DoesIntersect(const Ray &ray, double maxDistance);

	const std::vector<SimpleLight *> GetLightList() const { return m_SimpleLightList; }
	colorf GetAmbientLight() const { return m_AmbientLight; }

private:
	std::vector<Drawable *> m_DrawableList;;
	std::vector<SimpleLight *> m_SimpleLightList;
	colorf m_AmbientLight;
};


#endif

