#ifndef __SIMPLE_LIGHT_H
#define __SIMPLE_LIGHT_H

#include "Light.h"
#include <Color.h>
#include <libvec/Vector3.h>

class SimpleLight : public Light {
public:
	SimpleLight() {}
	SimpleLight(const Libvec::Vector3d &pos, colorf color, float falloff) :
		m_Position(pos), m_Color(color), m_Falloff(falloff) {}

	virtual ~SimpleLight() {}

	const Libvec::Vector3d &GetPos() const { return m_Position; }
	colorf GetColor() const { return m_Color; }
	float GetFalloff() const { return m_Falloff; }

private:
	Libvec::Vector3d m_Position;
	colorf m_Color;
	float m_Falloff;
};

#endif

