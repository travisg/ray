#ifndef __SCENE_H
#define __SCENE_H

#include <vector>
#include "Color.h"
#include <libmath/Vector3.h>

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

	const Math::Vector3d &GetCamera() const { return m_Camera; }
	const Math::Vector3d &GetTarget() const { return m_Target; }

private:
	std::vector<Drawable *> m_DrawableList;;
	std::vector<SimpleLight *> m_SimpleLightList;
	colorf m_AmbientLight;

	Math::Vector3d m_Camera;
	Math::Vector3d m_Target;
};


#endif

