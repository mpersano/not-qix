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
		auto read_vec3 = [&]() -> vec3
		{
			float x = a->read_float();
			float y = a->read_float();
			float z = a->read_float();

			return { x, y, z };
		};

		vec3 pos = read_vec3();
		vec3 normal = read_vec3();
		vec3 vnormal = read_vec3();
		vec3 color = read_vec3();

		verts_.push_back({ pos, normal, vnormal, color });
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
		gl_vertex()
		{ }

		gl_vertex(const vertex& v)
		: position { v.pos.x, v.pos.y, v.pos.z }
		, normal { v.normal.x, v.normal.y, v.normal.z }
		, vnormal { v.vnormal.x, v.vnormal.y, v.vnormal.z }
		, color { v.color.x, v.color.y, v.color.z }
		{ }

		GLfloat position[3];
		GLfloat normal[3];
		GLfloat vnormal[3];
		GLfloat color[3];
	};

	std::vector<gl_vertex> gl_verts(std::begin(verts_), std::end(verts_));

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

#define ENABLE_ATTRIB(location, size, field) \
	gl_check(glVertexAttribPointer(location, size, GL_FLOAT, GL_FALSE, sizeof(gl_vertex), reinterpret_cast<const GLvoid *>(offsetof(gl_vertex, field)))); \
	gl_check(glEnableVertexAttribArray(location));

	ENABLE_ATTRIB(0, 3, position)
	ENABLE_ATTRIB(1, 3, normal)
	ENABLE_ATTRIB(2, 3, vnormal)
	ENABLE_ATTRIB(3, 3, color)

#undef ENABLE_ATTRIB

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
