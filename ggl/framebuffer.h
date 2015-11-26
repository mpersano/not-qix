#pragma once

#include <ggl/gl.h>
#include <ggl/texture.h>

namespace ggl {

class texture;

class framebuffer
{
public:
	framebuffer(unsigned width, unsigned height);
	~framebuffer();

	void bind() const;
	static void unbind();

	const texture *get_texture() const
	{ return &texture_; }

private:
	void init_texture();

	texture texture_;
	GLuint fbo_id_;
};

}
