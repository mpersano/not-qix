#include <ggl/buffer_object.h>

namespace ggl {

gl_buffer_object::gl_buffer_object(GLenum target)
: target_ { target }
{
	glGenBuffers(1, &id_);
}

gl_buffer_object::~gl_buffer_object()
{
	glDeleteBuffers(1, &id_);
}

void
gl_buffer_object::bind()
{
	glBindBuffer(target_, id_);
}

void
gl_buffer_object::unbind()
{
	glBindBuffer(target_, 0);
}

void
gl_buffer_object::buffer_data(GLsizei size, const void *data, GLenum usage)
{
	glBufferData(target_, size, data, usage);
}

void *
gl_buffer_object::map(GLenum access)
{
	return glMapBuffer(target_, access);
}

void
gl_buffer_object::unmap()
{
	glUnmapBuffer(target_);
}

}
