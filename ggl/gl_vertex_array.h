#pragma once

#include <ggl/gl.h>
#include <ggl/noncopyable.h>

namespace ggl {

class gl_vertex_array : private noncopyable
{
public:
	gl_vertex_array();
	~gl_vertex_array();

	void bind() const;
	static void unbind();

private:
	GLuint id_;
};

}
