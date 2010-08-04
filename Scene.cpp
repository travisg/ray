#include <iostream>
#include <vector>	
#include <Scene.h>
#include <libmath/Vector3.h>
#include <drawables/Sphere.h>
#include <drawables/Plane.h>
#include <shaders/DefaultShader.h>
#include <lights/SimpleLight.h>

using Math::Vector3d;

Scene::Scene()
:	m_AmbientLight(0)
{
#if 0
#if 1
	for (int i=0; i < 100; i++) {
		DefaultShader *ds = new DefaultShader;
		Sphere *s = new Sphere(Vector3d((rand()%1000 - 500) / 10.0f, (rand()%1000 - 500) / 10.0f, (rand()%1000 - 500) / 10.0f), (rand()%200) / 10.0f + 1.0f);
		ds->SetDiffuseColor(colorf::RandColor());
		ds->SetShinyness(0.0f);
		s->SetShader(ShaderPtr(ds));
		m_DrawableList.push_back(s);
	}
#endif
	
	Sphere *s = new Sphere(Vector3d(0.0, 0.0, 0.0), 50.0);
	{
		DefaultShader *ds = new DefaultShader;
		ds->SetShinyness(0.0f);
		s->SetShader(ShaderPtr(ds));
	}
	m_DrawableList.push_back(s);

	Plane *p = new Plane(Ray(Vector3d(0.0, 0.0, -50.0), Vector3d(0.0, 0.0, 1.0)));
	{
		DefaultShader *ds = new DefaultShader;
		ds->SetShinyness(0.2f);
		p->SetShader(ShaderPtr(ds));
	}
	m_DrawableList.push_back(p);

	p = new Plane(Ray(Vector3d(-50.0, 0.0, 0.0), Vector3d(1.0, 0.0, 0.0)));
	{
		DefaultShader *ds = new DefaultShader;
		ds->SetShinyness(0.0f);
		p->SetShader(ShaderPtr(ds));
	}
	m_DrawableList.push_back(p);

	m_SimpleLightList.push_back(new SimpleLight(Vector3d(0.0, 0.0, 100.0), colorf(1.0, 0.0, 0.0), 100));
	m_SimpleLightList.push_back(new SimpleLight(Vector3d(100.0, 0.0, 5.0), colorf(0.0, 1.0, 0.0), 100));
	m_SimpleLightList.push_back(new SimpleLight(Vector3d(0.0, 100.0, 5.0), colorf(0.0, 0.0, 1.0), 100));
	m_SimpleLightList.push_back(new SimpleLight(Vector3d(0.0, 60.0, 60.0), colorf(1.0, 1.0, 1.0), 100));
#endif

	m_AmbientLight = colorf(0.0f, 0.0f, 0.0f);
	{
		Sphere *s = new Sphere(Vector3d(0.0, 0.0, 0.0), 8.0);
		{
			DefaultShader *ds = new DefaultShader;
			ds->SetShinyness(0.8f);
			s->SetShader(ShaderPtr(ds));
		}
		m_DrawableList.push_back(s);
	}

	// bottom
	{
		Plane *p = new Plane(Ray(Vector3d(0.0, 0.0, -10.0), Vector3d(0.0, 0.0, 1.0)));
		DefaultShader *ds = new DefaultShader;
		ds->SetDiffuseColor(colorf(1.0, 0, 0));
		p->SetShader(ShaderPtr(ds));
		m_DrawableList.push_back(p);
	}
	{
		Plane *p = new Plane(Ray(Vector3d(0.0, -10.0, 0.0), Vector3d(0.0, 1.0, 0.0)));
		DefaultShader *ds = new DefaultShader;
		ds->SetDiffuseColor(colorf(0, 1.0, 0));
		p->SetShader(ShaderPtr(ds));
		m_DrawableList.push_back(p);
	}
	// back
	{
		Plane *p = new Plane(Ray(Vector3d(-10.0, 0.0, 0.0), Vector3d(1.0, 0.0, 0.0)));
		DefaultShader *ds = new DefaultShader;
		ds->SetDiffuseColor(colorf(0, 0, 1.0));
		p->SetShader(ShaderPtr(ds));
		m_DrawableList.push_back(p);
	}

	m_SimpleLightList.push_back(new SimpleLight(Vector3d(0.0, 0.0, 100.0), colorf(1.0, 0.0, 0.0), 100));
	m_SimpleLightList.push_back(new SimpleLight(Vector3d(100.0, 0.0, 5.0), colorf(0.0, 1.0, 0.0), 100));
	m_SimpleLightList.push_back(new SimpleLight(Vector3d(0.0, 100.0, 5.0), colorf(0.0, 0.0, 1.0), 100));
	m_SimpleLightList.push_back(new SimpleLight(Vector3d(5.0, 10.0, 20), colorf(1.0, 1.0, 1.0), 100));
}

Scene::~Scene()
{
	// empty our list
	while(!m_DrawableList.empty()) {
		Drawable *d = m_DrawableList.back();
		m_DrawableList.pop_back();
		delete d;
	}
}

const Drawable *Scene::Intersect(const Ray &ray)
{
	bool hit = false;
	Vector3d closestPos = 0;
	Drawable *closest = NULL;

	for (std::vector<Drawable *>::const_iterator i = m_DrawableList.begin(); i != m_DrawableList.end(); i++) {
		Drawable *d = *i;

		Vector3d pos;
		Vector3d normal;
		if (d->Intersect(ray, pos, normal)) {
//				std::cout << "collision ray " << ray << " pos " << pos << std::endl;
			if (!hit || (pos - ray.origin).LengthSquared() < (closestPos - ray.origin).LengthSquared()) {
				closestPos = pos;
				closest = d;
			}
			hit = true;
		}
	}

	if (hit)
		return closest;

	return NULL;
}

bool Scene::DoesIntersect(const Ray &ray)
{
	for (std::vector<Drawable *>::const_iterator i = m_DrawableList.begin(); i != m_DrawableList.end(); i++) {
		Drawable *d = *i;

		if (d->Intersect(ray)) {
			return true;
		}
	}

	return false;
}

bool Scene::DoesIntersect(const Ray &ray, double maxDistance)
{
	for (std::vector<Drawable *>::const_iterator i = m_DrawableList.begin(); i != m_DrawableList.end(); i++) {
		Drawable *d = *i;

		Vector3d pos;
		Vector3d normal;
		if (d->Intersect(ray, pos, normal)) {
			if ((ray.origin - pos).Length() < maxDistance)
				return true;
		}
	}

	return false;
}

