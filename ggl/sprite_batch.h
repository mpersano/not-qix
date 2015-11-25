#pragma once

#include <vector>
#include <stack>
#include <memory>

#include <ggl/noncopyable.h>
#include <ggl/sprite.h>
#include <ggl/vec2.h>
#include <ggl/vec3.h>
#include <ggl/mat3.h>
#include <ggl/rgba.h>
#include <ggl/gl_vertex_array.h>
#include <ggl/gl_buffer.h>
#include <ggl/gl_program.h>

namespace ggl {

class texture;

struct bbox
{
	vec2f min, max;
};

struct quad
{
	vec2f p0, p1, p2, p3;
};

class sprite_batch : private noncopyable
{
public:
	sprite_batch();

	void set_viewport(const bbox& viewport);

	void begin();

	void translate(const vec2f& p);
	void translate(float x, float y);

	void scale(const vec2f& s);
	void scale(float sx, float sy);
	void scale(float s);

	void rotate(float a);

	void push_matrix();
	void pop_matrix();

	void set_color(const rgba& color);

	void draw(const texture *tex, const bbox& tex_coords, const bbox& dest_coords, float depth);
	void draw(const texture *tex, const bbox& tex_coords, const quad& dest_coords, float depth);

	void draw(const texture *tex0, const texture *tex1, const bbox& tex0_coords, const bbox& tex1_coords, const bbox& dest_coords, float depth);
	void draw(const texture *tex0, const texture *tex1, const bbox& tex0_coords, const bbox& tex1_coords, const quad& dest_coords, float depth);

	void end();

private:
	class impl;
	std::unique_ptr<impl> impl_;
};

}
