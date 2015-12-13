#pragma once

#include <vector>
#include <string>

#include <ggl/gl.h>
#include <ggl/noncopyable.h>

#include "vec3.h"

namespace ggl {

class mesh : private noncopyable
{
public:
	mesh(const std::string& path);
	~mesh();

	void draw() const;

	void load();
	void unload();

private:
	void load(const std::string& path);

	struct vertex {
		vec3 pos;
		vec3 normal;
		vec3 vnormal;
		vec3 color;
	};
	std::vector<vertex> verts_;

	struct triangle {
		int v0, v1, v2;
	};
	std::vector<triangle> tris_;

	GLuint vertex_buffer_, index_buffer_;
	GLuint vao_id_;
};

}
