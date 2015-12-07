#pragma once

// this is a disgrace.

#include <cassert>
#include <vector>

#include <ggl/gl.h>
#include <ggl/gl_vertex_array.h>
#include <ggl/resources.h>
#include <ggl/gl_check.h>
#include <ggl/rgba.h>
#include <ggl/render.h>

namespace ggl {

template <typename VertexType, int VertexSize>
struct vertex_flat
{
	VertexType position[VertexSize];
};

template <typename VertexType, int VertexSize, typename TexCoordType, int TexCoordSize>
struct vertex_texcoord
{
	VertexType position[VertexSize];
	TexCoordType texcoord[TexCoordSize];
};

namespace detail {

template <typename GLType>
struct gltype_to_glenum;

template <>
struct gltype_to_glenum<GLbyte>
{
	static const GLenum type = GL_BYTE;
};

template <>
struct gltype_to_glenum<GLubyte>
{
	static const GLenum type = GL_UNSIGNED_BYTE;
};

template <>
struct gltype_to_glenum<GLshort>
{
	static const GLenum type = GL_SHORT;
};

template <>
struct gltype_to_glenum<GLushort>
{
	static const GLenum type = GL_UNSIGNED_SHORT;
};

template <>
struct gltype_to_glenum<GLfloat>
{
	static const GLenum type = GL_FLOAT;
};

template <typename Vertex>
struct vertex_traits;

template <typename VertexType, int VertexSize>
struct vertex_traits<vertex_flat<VertexType, VertexSize>>
{
	static void enable_vertex_attribs(const vertex_flat<VertexType, VertexSize> *p)
	{
		gl_check(glVertexAttribPointer(0, VertexSize, gltype_to_glenum<VertexType>::type, GL_FALSE, sizeof(*p), p->position));
		gl_check(glEnableVertexAttribArray(0));
	}

	static void disable_vertex_attribs()
	{
		gl_check(glDisableVertexAttribArray(0));
	}
};

template <typename VertexType, int VertexSize, typename TexCoordType, int TexCoordSize>
struct vertex_traits<vertex_texcoord<VertexType, VertexSize, TexCoordType, TexCoordSize>>
{
	static void enable_vertex_attribs(const vertex_texcoord<VertexType, VertexSize, TexCoordType, TexCoordSize> *p)
	{
		gl_check(glVertexAttribPointer(0, VertexSize, gltype_to_glenum<VertexType>::type, GL_FALSE, sizeof(*p), p->position));
		gl_check(glEnableVertexAttribArray(0));

		gl_check(glVertexAttribPointer(1, TexCoordSize, gltype_to_glenum<TexCoordType>::type, GL_FALSE, sizeof(*p), p->texcoord));
		gl_check(glEnableVertexAttribArray(1));
	}

	static void disable_vertex_attribs()
	{
		gl_check(glDisableVertexAttribArray(1));
		gl_check(glDisableVertexAttribArray(0));
	}
};

} // detail

namespace render {
class renderer;
};

template <typename VertexType>
class vertex_array : public std::vector<VertexType>
{
public:
	vertex_array()
	{ }

	vertex_array(std::initializer_list<VertexType> l)
	: std::vector<VertexType>(l)
	{ }

	void draw(GLenum mode) const
	{
		if (!this->empty()) {
			detail::vertex_traits<VertexType>::enable_vertex_attribs(&this->front());
			gl_check(glDrawArrays(mode, 0, this->size()));
			detail::vertex_traits<VertexType>::disable_vertex_attribs();
		}
	}
};

template <typename VertexType, int VertexSize>
using vertex_array_flat = vertex_array<vertex_flat<VertexType, VertexSize>>;

template <typename VertexType, int VertexSize, typename TexCoordType, int TexCoordSize>
using vertex_array_texcoord = vertex_array<vertex_texcoord<VertexType, VertexSize, TexCoordType, TexCoordSize>>;

} // ggl
