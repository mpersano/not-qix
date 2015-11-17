#include <ggl/gl_check.h>
#include <ggl/gl_vertex_array.h>

namespace ggl {

gl_vertex_array::gl_vertex_array()
{
	gl_check(glGenVertexArrays(1, &id_));
}

gl_vertex_array::~gl_vertex_array()
{
	gl_check(glDeleteVertexArrays(1, &id_));
}

void
gl_vertex_array::bind()
{
	gl_check(glBindVertexArray(id_));
}

void
gl_vertex_array::unbind()
{
	gl_check(glBindVertexArray(0));
}

}
