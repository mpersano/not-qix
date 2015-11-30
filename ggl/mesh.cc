#include <ggl/panic.h>
#include <ggl/core.h>
#include <ggl/asset.h>
#include <ggl/gl_check.h>

#include "mesh.h"

namespace ggl {

mesh::mesh(const std::string& path)
{
	load(path);
	load();
}

mesh::~mesh()
{
	unload();
}

void
mesh::load(const std::string& path)
{
	auto a = g_core->get_asset(path);

	uint32_t sig = a->read_uint32();
	if (sig != ('M' | ('E' << 8) | ('S' << 16) | ('H' << 24)))
		panic("%s: not a mesh", path.c_str());

	uint16_t num_verts = a->read_uint16();
	verts_.reserve(num_verts);

	for (unsigned i = 0; i < num_verts; i++) {
		vec3 pos;
		pos.x = a->read_float();
		pos.y = a->read_float();
		pos.z = a->read_float();

		vec3 normal;
		normal.x = a->read_float();
		normal.y = a->read_float();
		normal.z = a->read_float();

		verts_.push_back({ pos, normal });
	}

	uint16_t num_tris = a->read_uint16();
	tris_.reserve(num_tris);

	for (unsigned i = 0; i < num_tris; i++) {
		int v0 = a->read_uint16();
		int v1 = a->read_uint16();
		int v2 = a->read_uint16();
		tris_.push_back({ v0, v1, v2 });
	}
}

void
mesh::load()
{
	struct gl_vertex {
		GLfloat position[3];
		GLfloat normal[3];
	};

	std::vector<gl_vertex> gl_verts(verts_.size());

	std::transform(
		std::begin(verts_),
		std::end(verts_),
		std::begin(gl_verts),
		[](const vertex& v) -> gl_vertex
		{ return { { v.pos.x, v.pos.y, v.pos.z }, { v.normal.x, v.normal.y, v.normal.z } }; });

	gl_check(glGenBuffers(1, &vertex_buffer_));

	gl_check(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_));
	gl_check(glBufferData(GL_ARRAY_BUFFER, gl_verts.size()*sizeof(gl_vertex), &gl_verts[0], GL_STATIC_DRAW));
	gl_check(glBindBuffer(GL_ARRAY_BUFFER, 0));

	gl_check(glGenBuffers(1, &index_buffer_));

	std::vector<GLushort> indices;
	indices.reserve(3*tris_.size());

	for (auto& t : tris_) {
		indices.push_back(t.v0);
		indices.push_back(t.v1);
		indices.push_back(t.v2);
	}

	gl_check(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_));
	gl_check(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLushort), &indices[0], GL_STATIC_DRAW));
	gl_check(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

	gl_check(glGenVertexArrays(1, &vao_id_));
	gl_check(glBindVertexArray(vao_id_));

	gl_check(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_));

	gl_check(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(gl_vertex), reinterpret_cast<const GLvoid *>(offsetof(gl_vertex, position))));
	gl_check(glEnableVertexAttribArray(0));

	gl_check(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(gl_vertex), reinterpret_cast<const GLvoid *>(offsetof(gl_vertex, normal))));
	gl_check(glEnableVertexAttribArray(1));

	gl_check(glBindBuffer(GL_ARRAY_BUFFER, 0));
	gl_check(glBindVertexArray(0));
}

void
mesh::unload()
{
	gl_check(glDeleteVertexArrays(1, &vao_id_));
	gl_check(glDeleteBuffers(1, &vertex_buffer_));
	gl_check(glDeleteBuffers(1, &index_buffer_));
}

void
mesh::draw() const
{
	gl_check(glBindVertexArray(vao_id_));
	gl_check(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_));
	gl_check(glDrawElements(GL_TRIANGLES, 3*tris_.size(), GL_UNSIGNED_SHORT, 0));
	gl_check(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	gl_check(glBindVertexArray(0));
}

}
