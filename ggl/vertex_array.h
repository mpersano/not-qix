#pragma once

#include <GL/glew.h>

#include <vector>

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
struct gltype_to_glenum<GLint>
{
	static const GLenum type = GL_INT;
};

template <>
struct gltype_to_glenum<GLuint>
{
	static const GLenum type = GL_UNSIGNED_INT;
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

} // detail

template <typename VertexType>
class vertex_array_base
{
public:
	vertex_array_base() = default;

	void clear()
	{ verts_.clear(); }

	void add_vertex(const VertexType& v)
	{ verts_.push_back(v); }

	size_t get_num_verts() const
	{ return verts_.size(); }

protected:
	std::vector<VertexType> verts_;
};

template <typename IndexType, typename VertexType>
class indexed_vertex_array_base : public vertex_array_base<VertexType>
{
public:
	using base_type = vertex_array_base<VertexType>;

	void clear()
	{
		base_type::clear();
		indices_.clear();
	}

	void add_index(const IndexType& index)
	{ indices_.push_back(index); }

	size_t get_num_indices() const
	{ return indices_.size(); }

protected:
	std::vector<IndexType> indices_;
};

template <typename VertexType>
class vertex_array : public vertex_array_base<VertexType>
{
public:
	void draw(GLenum mode) const
	{
		detail::client_state<VertexType> state(&this->verts_[0]);
		glDrawArrays(mode, 0, this->verts_.size());
	}
};

template <typename IndexType, typename VertexType>
class indexed_vertex_array : public indexed_vertex_array_base<IndexType, VertexType>
{
public:
	void draw(GLenum mode) const
	{
		detail::client_state<VertexType> state(&this->verts_[0]);
		glDrawElements(mode, this->indices_.size(), detail::gltype_to_glenum<IndexType>::type, &this->indices_[0]);
	}
};

} // ggl
