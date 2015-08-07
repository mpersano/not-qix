#pragma once

#include <GL/glew.h>

#include "noncopyable.h"
#include "image.h"

namespace ggl {

class texture : private noncopyable
{
public:
	texture(const image& pm);

	~texture()
	{ glDeleteTextures(1, &id_); }

	void bind() const
	{ glBindTexture(GL_TEXTURE_2D, id_); }

	void set_wrap_s(GLint wrap) const
	{ set_parameter(GL_TEXTURE_WRAP_S, wrap); }

	void set_wrap_t(GLint wrap) const
	{ set_parameter(GL_TEXTURE_WRAP_T, wrap); }

	void set_mag_filter(GLint filter) const
	{ set_parameter(GL_TEXTURE_MAG_FILTER, filter); }

	void set_min_filter(GLint filter) const
	{ set_parameter(GL_TEXTURE_MIN_FILTER, filter); }

	void set_parameter(GLenum name, GLint value) const
	{
		bind();
		glTexParameteri(GL_TEXTURE_2D, name, value);
	}

	static void set_env_mode(GLint mode)
	{ glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode); }

	size_t orig_width, width;
	size_t orig_height, height;
	image::pixel_type pixel_type;
	std::vector<uint8_t> data;

private:
	GLuint id_;
};

}
