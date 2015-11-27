#include <ggl/gl_check.h>
#include <ggl/texture.h>
#include <ggl/framebuffer.h>

namespace ggl {

framebuffer::framebuffer(unsigned width, unsigned height)
: width_ { width }
, height_ { height }
{
	// initialize texture

	// note to self: non-power of 2 textures are allowed on es >2.0 if
	// GL_MIN_FILTER is set to a function that doesn't require mipmaps
	// and texture wrap is set to GL_CLAMP_TO_EDGE

	gl_check(glGenTextures(1, &texture_id_));

	bind_texture();

	gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	gl_check(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));

	// initialize framebuffer

	gl_check(glGenFramebuffers(1, &fbo_id_));

	bind();
	gl_check(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id_, 0));
	unbind();
}

framebuffer::~framebuffer()
{
	gl_check(glDeleteFramebuffers(1, &fbo_id_));
}

void
framebuffer::bind() const
{
	gl_check(glBindFramebuffer(GL_FRAMEBUFFER, fbo_id_));
}

void
framebuffer::unbind()
{
	gl_check(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void
framebuffer::bind_texture() const
{
	gl_check(glBindTexture(GL_TEXTURE_2D, texture_id_));
}

}
