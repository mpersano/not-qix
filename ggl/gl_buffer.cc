#include <ggl/gl_buffer.h>

namespace ggl {

gl_buffer::gl_buffer(GLenum target)
: target_ { target }
{
	glGenBuffers(1, &id_);
}

gl_buffer::~gl_buffer()
{
	glDeleteBuffers(1, &id_);
}

void
gl_buffer::bind()
{
	glBindBuffer(target_, id_);
}

void
gl_buffer::unbind()
{
	glBindBuffer(target_, 0);
}

void
gl_buffer::buffer_data(GLsizei size, const void *data, GLenum usage)
{
	glBufferData(target_, size, data, usage);
}

void *
gl_buffer::map(GLenum access)
{
	return glMapBuffer(target_, access);
}

void
gl_buffer::unmap()
{
	glUnmapBuffer(target_);
}

}
