#include <vector>	
#include <Scene.h>
#include <math/Vector3.h>
#include <Sphere.h>

using Math::Vector3;

Scene::Scene()
{
#if 1
	for (int i=0; i < 100; i++) {
		slist.push_back(new Sphere(Vector3((rand()%1000 - 500) / 10.0f, (rand()%1000 - 500) / 10.0f, (rand()%1000 - 500) / 10.0f), (rand()%200) / 10.0f));
	}
#endif
	
	slist.push_back(new Sphere(Vector3(0.0, 0.0, 0.0), 0.25));
	slist.push_back(new Sphere(Vector3(0.5, 0.0, 0.0), 0.25));
	slist.push_back(new Sphere(Vector3(0.5, 0.5, 0.0), 0.25));
	slist.push_back(new Sphere(Vector3(0.5, 0.6, 0.5), 0.10));
}

Scene::~Scene()
{
}

const Sphere *Scene::Intersect(const Ray &ray)
{
	bool hit = false;
	Vector3 closestPos;
	Sphere *closestSphere = NULL;

	for (std::vector<Sphere *>::const_iterator i = slist.begin(); i != slist.end(); i++) {
		Sphere *s = *i;

		Vector3 pos;
		Vector3 normal;
		if (s->Intersect(ray, pos, normal)) {
//				std::cout << "collision ray " << ray << " pos " << pos << std::endl;
			if (!hit || (pos - ray.origin).LengthSquared() < (closestPos - ray.origin).LengthSquared()) {
				closestPos = pos;
				closestSphere = s;
			}
			hit = true;
		}
	}

	if (hit)
		return closestSphere;

	return NULL;
}

bool Scene::DoesIntersect(const Ray &ray)
{
	for (std::vector<Sphere *>::const_iterator i = slist.begin(); i != slist.end(); i++) {
		Sphere *s = *i;

		if (s->Intersect(ray)) {
			return true;
		}
	}

	return false;
}

