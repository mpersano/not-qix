#pragma once

#include <vector>

#include <ggl/gl.h>

namespace ggl {

template <typename VertexType, int VertexSize>
struct vertex_flat
{
	VertexType pos[VertexSize];
};

template <typename VertexType, int VertexSize, typename TexCoordType, int TexCoordSize>
struct vertex_texcoord
{
	VertexType pos[VertexSize];
	TexCoordType texcoord[TexCoordSize];
};

template <typename VertexType, int VertexSize, typename TexCoord0Type, int TexCoord0Size, typename TexCoord1Type, int TexCoord1Size>
struct vertex_multitexcoord
{
	VertexType pos[VertexSize];
	TexCoord0Type texcoord0[TexCoord0Size];
	TexCoord1Type texcoord1[TexCoord1Size];
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
struct client_state;

template <typename VertexType, int VertexSize>
struct client_state<vertex_flat<VertexType, VertexSize>>
{
	client_state(const vertex_flat<VertexType, VertexSize> *verts)
	{
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(VertexSize, gltype_to_glenum<VertexType>::type, sizeof *verts, verts->pos);
	}

	~client_state()
	{
		glDisableClientState(GL_VERTEX_ARRAY);
	}
};

template <typename VertexType, int VertexSize, typename TexCoordType, int TexCoordSize>
struct client_state<vertex_texcoord<VertexType, VertexSize, TexCoordType, TexCoordSize>>
{
	client_state(const vertex_texcoord<VertexType, VertexSize, TexCoordType, TexCoordSize> *verts)
	{
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(VertexSize, gltype_to_glenum<VertexType>::type, sizeof *verts, verts->pos);

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(TexCoordSize, gltype_to_glenum<TexCoordType>::type, sizeof *verts, verts->texcoord);
	}

	~client_state()
	{
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
	}
};

template <typename VertexType, int VertexSize, typename TexCoord0Type, int TexCoord0Size, typename TexCoord1Type, int TexCoord1Size>
struct client_state<vertex_multitexcoord<VertexType, VertexSize, TexCoord0Type, TexCoord0Size, TexCoord1Type, TexCoord1Size>>
{
	client_state(const vertex_multitexcoord<VertexType, VertexSize, TexCoord0Type, TexCoord0Size, TexCoord1Type, TexCoord1Size> *verts)
	{
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(VertexSize, gltype_to_glenum<VertexType>::type, sizeof *verts, verts->pos);

		glClientActiveTexture(GL_TEXTURE0);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(TexCoord0Size, gltype_to_glenum<TexCoord0Type>::type, sizeof *verts, verts->texcoord0);

		glClientActiveTexture(GL_TEXTURE1);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(TexCoord1Size, gltype_to_glenum<TexCoord1Type>::type, sizeof *verts, verts->texcoord1);
	}

	~client_state()
	{
		glClientActiveTexture(GL_TEXTURE1);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		glClientActiveTexture(GL_TEXTURE0);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		glDisableClientState(GL_VERTEX_ARRAY);
	}
};

} // detail

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
		detail::client_state<VertexType> state(&this->front());
		glDrawArrays(mode, 0, this->size());
	}
};

template <typename VertexType, int VertexSize>
using vertex_array_flat = vertex_array<vertex_flat<VertexType, VertexSize>>;

template <typename VertexType, int VertexSize, typename TexCoordType, int TexCoordSize>
using vertex_array_texcoord = vertex_array<vertex_texcoord<VertexType, VertexSize, TexCoordType, TexCoordSize>>;

template <typename VertexType, int VertexSize, typename TexCoord0Type, int TexCoord0Size, typename TexCoord1Type, int TexCoord1Size>
using vertex_array_multitexcoord = vertex_array<vertex_multitexcoord<VertexType, VertexSize, TexCoord0Type, TexCoord0Size, TexCoord1Type, TexCoord1Size>>;

} // ggl
