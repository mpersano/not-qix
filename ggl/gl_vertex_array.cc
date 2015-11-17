#include <ggl/gl_vertex_array.h>

namespace ggl {

gl_vertex_array::gl_vertex_array()
{
	glGenVertexArrays(1, &id_);
}

gl_vertex_array::~gl_vertex_array()
{
	glDeleteVertexArrays(1, &id_);
}

void
gl_vertex_array::bind()
{
	glBindVertexArray(id_);
}

void
gl_vertex_array::unbind()
{
	glBindVertexArray(0);
}

}
