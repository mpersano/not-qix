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
gl_buffer::bind() const
{
	gl_check(glBindBuffer(target_, id_));
}

void
gl_buffer::unbind() const
{
	gl_check(glBindBuffer(target_, 0));
}

void
gl_buffer::buffer_data(GLsizei size, const void *data, GLenum usage) const
{
	gl_check(glBufferData(target_, size, data, usage));
}

void *
gl_buffer::map_range(GLintptr offset, GLsizei length, GLbitfield access) const
{
	return gl_check_r(glMapBufferRange(target_, offset, length, access));
}

void
gl_buffer::unmap() const
{
	gl_check(glUnmapBuffer(target_));
}

}
