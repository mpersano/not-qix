#include <ggl/gl_check.h>
#include <ggl/texture.h>
#include <ggl/framebuffer.h>

namespace ggl {

framebuffer::framebuffer(unsigned width, unsigned height)
: texture_ { width, height, pixel_type::RGB_ALPHA }
{
	gl_check(glGenFramebuffers(1, &fbo_id_));

	bind();
	gl_check(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_.id_, 0));
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

}
