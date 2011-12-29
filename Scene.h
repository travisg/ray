#ifndef __SCENE_H
#define __SCENE_H

#include <vector>
#include "Color.h"
#include <libvec/Vector3.h>

class Drawable;
struct Ray;
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

	const Libvec::Vector3d &GetCamera() const { return m_Camera; }
	const Libvec::Vector3d &GetTarget() const { return m_Target; }

private:
	std::vector<Drawable *> m_DrawableList;;
	std::vector<SimpleLight *> m_SimpleLightList;
	colorf m_AmbientLight;

	Libvec::Vector3d m_Camera;
	Libvec::Vector3d m_Target;
};


#endif

