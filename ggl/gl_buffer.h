#pragma once

#include <ggl/gl.h>
#include <ggl/noncopyable.h>

namespace ggl {

class gl_buffer : private noncopyable
{
public:
	gl_buffer(GLenum target);
	~gl_buffer();

	void bind() const;
	void unbind() const;

	void buffer_data(GLsizei size, const void *data, GLenum usage) const;

	void *map_range(GLintptr offset, GLsizei length, GLbitfield access) const;
	void unmap() const;

// private:
	GLenum target_;
	GLuint id_;
};

};
