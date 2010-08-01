#ifndef __DEFAULT_SHADER_H
#define __DEFAULT_SHADER_H

#include "Shader.h"
#include "Color.h"

class DefaultShader : public Shader {
public:
	DefaultShader();
	virtual ~DefaultShader();

	virtual colorf Run(const ShaderArgs &args);

	void SetShinyness(float shiny) { if (shiny > 1.0f) shiny = 1.0f; if (shiny < 0) shiny = 0; m_Shinyness = shiny; }
	void SetDiffuseColor(colorf diffuse) { m_DiffuseColor = diffuse; }

private:
	float   m_Shinyness;
	colorf	m_DiffuseColor;
};

#endif

