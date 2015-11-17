#pragma once

#include <ggl/gl.h>
#include <ggl/noncopyable.h>

namespace ggl {

class gl_buffer : private noncopyable
{
public:
	gl_buffer(GLenum target);
	~gl_buffer();

	void bind();
	void unbind();

	void buffer_data(GLsizei size, const void *data, GLenum usage);

	void *map(GLenum access);
	void unmap();

private:
	GLenum target_;
	GLuint id_;
};

};
