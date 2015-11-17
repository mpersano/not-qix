#include <ggl/gl_check.h>
#include <ggl/gl_buffer.h>

namespace ggl {

gl_buffer::gl_buffer(GLenum target)
: target_ { target }
{
	gl_check(glGenBuffers(1, &id_));
}

gl_buffer::~gl_buffer()
{
	gl_check(glDeleteBuffers(1, &id_));
}

void
gl_buffer::bind()
{
	gl_check(glBindBuffer(target_, id_));
}

void
gl_buffer::unbind()
{
	gl_check(glBindBuffer(target_, 0));
}

void
gl_buffer::buffer_data(GLsizei size, const void *data, GLenum usage)
{
	gl_check(glBufferData(target_, size, data, usage));
}

void *
gl_buffer::map(GLenum access)
{
	void *rv;
	gl_check(rv = glMapBuffer(target_, access));
	return rv;
}

void
gl_buffer::unmap()
{
	gl_check(glUnmapBuffer(target_));
}

}
