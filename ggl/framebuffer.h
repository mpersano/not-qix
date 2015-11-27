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

	void bind_texture() const;

	unsigned get_width() const
	{ return width_; }

	unsigned get_height() const
	{ return height_; }

private:
	void init_texture();

	unsigned width_, height_;
	GLuint texture_id_;
	GLuint fbo_id_;
};

}
