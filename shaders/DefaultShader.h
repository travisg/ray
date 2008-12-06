#ifndef __DEFAULT_SHADER_H
#define __DEFAULT_SHADER_H

#include "Shader.h"

class DefaultShader : public Shader {
public:
	DefaultShader();
	virtual ~DefaultShader();

	virtual colorf Run(const ShaderArgs &args);
};

#endif

