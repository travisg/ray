/*
 * Copyright (c) 2008-2011 Travis Geiselbrecht
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <cstdio>
#include <iostream>
#include <vector>	
#include <Scene.h>
#include <libvec/Vector3.h>
#include <libvec/Random.h>
#include <drawables/Sphere.h>
#include <drawables/Plane.h>
#include <drawables/Triangle.h>
#include <drawables/Mesh.h>
#include <shaders/DefaultShader.h>
#include <lights/SimpleLight.h>
#include <mesh/OBJLoader.h>

using Libvec::Vector3d;

Scene::Scene()
:	m_AmbientLight(0)
{
#if 0
#if 1
	for (int i=0; i < 100; i++) {
		DefaultShader *ds = new DefaultShader;
		Sphere *s = new Sphere(Vector3d(Libvec::Random(-50, 50), Libvec::Random(-50, 50), Libvec::Random(-50, 50)), Libvec::Random(1, 20));
		ds->SetDiffuseColor(colorf::RandColor());
		ds->SetShinyness(0.0f);
		s->SetShader(ShaderPtr(ds));
		s->Prepare();
		m_DrawableList.push_back(s);
	}
#endif
	
	Sphere *s = new Sphere(Vector3d(0.0, 0.0, 0.0), 50.0);
	{
		DefaultShader *ds = new DefaultShader;
		ds->SetShinyness(0.0f);
		s->SetShader(ShaderPtr(ds));
		s->Prepare();
	}
	m_DrawableList.push_back(s);

	Plane *p = new Plane(Ray(Vector3d(0.0, 0.0, -50.0), Vector3d(0.0, 0.0, 1.0)));
	{
		DefaultShader *ds = new DefaultShader;
		ds->SetShinyness(0.2f);
		p->SetShader(ShaderPtr(ds));
		p->Prepare();
	}
	m_DrawableList.push_back(p);

	p = new Plane(Ray(Vector3d(-50.0, 0.0, 0.0), Vector3d(1.0, 0.0, 0.0)));
	{
		DefaultShader *ds = new DefaultShader;
		ds->SetShinyness(0.0f);
		p->SetShader(ShaderPtr(ds));
		p->Prepare();
	}
	m_DrawableList.push_back(p);

	m_SimpleLightList.push_back(new SimpleLight(Vector3d(0.0, 0.0, 100.0), colorf(1.0, 0.0, 0.0), 100));
	m_SimpleLightList.push_back(new SimpleLight(Vector3d(100.0, 0.0, 5.0), colorf(0.0, 1.0, 0.0), 100));
	m_SimpleLightList.push_back(new SimpleLight(Vector3d(0.0, 100.0, 5.0), colorf(0.0, 0.0, 1.0), 100));
	m_SimpleLightList.push_back(new SimpleLight(Vector3d(0.0, 60.0, 60.0), colorf(1.0, 1.0, 1.0), 100));
#endif
#if 0
	m_AmbientLight = colorf(0.0f, 0.0f, 0.0f);
	{
		Sphere *s = new Sphere(Vector3d(10.0, 10.0, 10.0), 8.0);
		{
			DefaultShader *ds = new DefaultShader;
			ds->SetShinyness(0.8f);
			s->SetShader(ShaderPtr(ds));
			s->Prepare();
		}
		m_DrawableList.push_back(s);
	}

	// bottom
	{
		Plane *p = new Plane(Ray(Vector3d(0.0, 0.0, 0.0), Vector3d(0.0, 0.0, 1.0)));
		DefaultShader *ds = new DefaultShader;
		ds->SetDiffuseColor(colorf(1.0, 0.2, 0.2));
		ds->SetShinyness(0.5f);
		p->SetShader(ShaderPtr(ds));
		p->Prepare();
		m_DrawableList.push_back(p);
	}
	{
		Plane *p = new Plane(Ray(Vector3d(0.0, 0.0, 0.0), Vector3d(0.0, 1.0, 0.0)));
		DefaultShader *ds = new DefaultShader;
		ds->SetDiffuseColor(colorf(0.2, 1.0, 0.2));
		ds->SetShinyness(0.5f);
		p->SetShader(ShaderPtr(ds));
		p->Prepare();
		m_DrawableList.push_back(p);
	}
	// back
	{
		Plane *p = new Plane(Ray(Vector3d(0.0, 0.0, 0.0), Vector3d(1.0, 0.0, 0.0)));
		DefaultShader *ds = new DefaultShader;
		ds->SetDiffuseColor(colorf(0.2, 0.2, 1.0));
		ds->SetShinyness(0.5f);
		p->SetShader(ShaderPtr(ds));
		p->Prepare();
		m_DrawableList.push_back(p);
	}

//	m_SimpleLightList.push_back(new SimpleLight(Vector3d(1.0, 1.0, 100.0), colorf(1.0, 0.0, 0.0), 100));
//	m_SimpleLightList.push_back(new SimpleLight(Vector3d(100.0, 1.0, 5.0), colorf(0.0, 1.0, 0.0), 100));
//	m_SimpleLightList.push_back(new SimpleLight(Vector3d(1.0, 100.0, 5.0), colorf(0.0, 0.0, 1.0), 100));
	m_SimpleLightList.push_back(new SimpleLight(Vector3d(20.0, 10.0, 20), colorf(1.0, 1.0, 1.0), 100));
#endif

#if 1
	m_AmbientLight = colorf(0.0f, 0.0f, 0.2f);
	if (0) {
		Triangle *t = new Triangle(Vector3d(0.0, 0.0, 10.0), Vector3d(20.0, 0.0, 0.0), Vector3d(0.0, 20.0, 0.0));
		DefaultShader *ds = new DefaultShader;
		ds->SetDiffuseColor(colorf(0.2, 0.2, 1.0));
		ds->SetShinyness(0.5f);
		t->SetShader(ShaderPtr(ds));
		t->Prepare();
		m_DrawableList.push_back(t);
	}
	if (1) {
		Plane *p = new Plane(Ray(Vector3d(0.0, 0.0, -3.0), Vector3d(0.0, 0.0, 1.0)));
		DefaultShader *ds = new DefaultShader;
		ds->SetDiffuseColor(colorf(1.0, 0.0, 0.0));
		ds->SetShinyness(0.2f);
		p->SetShader(ShaderPtr(ds));
		p->Prepare();
		m_DrawableList.push_back(p);
	}

//	m_SimpleLightList.push_back(new SimpleLight(Vector3d(1.0, 1.0, 100.0), colorf(1.0, 0.0, 0.0), 100));
//	m_SimpleLightList.push_back(new SimpleLight(Vector3d(100.0, 1.0, 5.0), colorf(0.0, 1.0, 0.0), 100));
	m_SimpleLightList.push_back(new SimpleLight(Vector3d(1.0, 100.0, 5.0), colorf(0.0, 0.0, 1.0), 100));
//	m_SimpleLightList.push_back(new SimpleLight(Vector3d(20.0, 10.0, 20), colorf(1.0, 1.0, 1.0), 100));
#endif

	if (1) {
		Geometry *g;
		std::cout << "loading cbox" << std::endl;
		FILE *fp = fopen("resources/cbox.obj", "r");
		if (obj_load(fp, &g) < 0) {
			std::cerr << "error loading mesh" << std::endl;
		} else {
			g->Dump();

			MeshDrawable *md = new MeshDrawable(boost::shared_ptr<Geometry>(g));
			DefaultShader *ds = new DefaultShader;
			ds->SetDiffuseColor(colorf(1.0, 1.0, 0.0));
			ds->SetShinyness(0.0f);
			md->SetShader(ShaderPtr(ds));
			md->Prepare();
			m_DrawableList.push_back(md);
		}
		fclose(fp);
	}
	if (1) {
		Geometry *g;
		std::cout << "loading dude" << std::endl;
		FILE *fp = fopen("resources/dude.obj", "r");
		if (obj_load(fp, &g) < 0) {
			std::cerr << "error loading mesh" << std::endl;
		} else {
			g->Dump();
			MeshDrawable *md = new MeshDrawable(boost::shared_ptr<Geometry>(g));
			DefaultShader *ds = new DefaultShader;
			ds->SetDiffuseColor(colorf(1.0, 0.0, 1.0));
			ds->SetShinyness(0.0f);
			md->SetShader(ShaderPtr(ds));
			md->Prepare();
			m_DrawableList.push_back(md);
		}
		fclose(fp);
	}
	if (0) {
		Geometry *g;
		std::cout << "loading plane" << std::endl;
		FILE *fp = fopen("resources/plane.obj", "r");
		if (obj_load(fp, &g) < 0) {
			std::cerr << "error loading mesh" << std::endl;
		} else {
			g->Dump();
			MeshDrawable *md = new MeshDrawable(boost::shared_ptr<Geometry>(g));
			DefaultShader *ds = new DefaultShader;
			ds->SetDiffuseColor(colorf(0.0, 0.0, 1.0));
			ds->SetShinyness(0.0f);
			md->SetShader(ShaderPtr(ds));
			md->Prepare();
			m_DrawableList.push_back(md);
		}
		fclose(fp);
	}
	if (1) {
		Geometry *g;
		std::cout << "loading blob" << std::endl;
		FILE *fp = fopen("resources/blob.obj", "r");
		if (obj_load(fp, &g) < 0) {
			std::cerr << "error loading mesh" << std::endl;
		} else {
			g->Dump();
			MeshDrawable *md = new MeshDrawable(boost::shared_ptr<Geometry>(g));
			DefaultShader *ds = new DefaultShader;
			ds->SetDiffuseColor(colorf(1.0, 0.5, 0.0));
			ds->SetShinyness(0.0f);
			md->SetShader(ShaderPtr(ds));
			md->Prepare();
			m_DrawableList.push_back(md);
		}
		fclose(fp);
	}

//	m_SimpleLightList.push_back(new SimpleLight(Vector3d(0.0, 0.0, 20), colorf(1.0, 1.0, 1.0), 100));

	m_Camera = Libvec::Vector3d(3.0f, 3.0f, 3.0f);
	m_SimpleLightList.push_back(new SimpleLight(Vector3d(1.0f, 4.0f, 7.0f), colorf(0.5, 1.0, 0.5), 100));
//	m_Camera = Libvec::Vector3d(80.0f, 80.0f, 10.0f);

	m_Target = Libvec::Vector3d(0.0f, 0.0f, 0.0f);
}

Scene::~Scene()
{
	// empty our lists
	while(!m_DrawableList.empty()) {
		Drawable *d = m_DrawableList.back();
		m_DrawableList.pop_back();
		delete d;
	}

	while(!m_SimpleLightList.empty()) {
		SimpleLight *l = m_SimpleLightList.back();
		m_SimpleLightList.pop_back();
		delete l;
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

