#pragma once

#include <vector>
#include <stack>
#include <memory>

#include <ggl/noncopyable.h>
#include <ggl/sprite.h>
#include <ggl/vec2.h>
#include <ggl/mat3.h>
#include <ggl/rgba.h>
#include <ggl/gl_vertex_array.h>
#include <ggl/gl_buffer.h>

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

	void begin();

	void translate(const vec2f& p);
	void translate(float x, float y);

	void scale(float s);
	void scale(float sx, float sy);

	void rotate(float a);

	void push_matrix();
	void pop_matrix();

	void set_color(const rgba& color);

	void draw(const texture *tex, const bbox& tex_coords, const bbox& dest_coords, float depth);
	void draw(const texture *tex, const bbox& tex_coords, const quad& dest_coords, float depth);

	void draw(const texture *tex0, const texture *tex1, const bbox& tex_coords0, const bbox& tex_coords1, const bbox& dest_coords, float depth);
	void draw(const texture *tex0, const texture *tex1, const bbox& tex_coords0, const bbox& tex_coords1, const quad& dest_coords, float depth);

	void end();

private:
	struct sprite_info
	{
		const texture *tex0, *tex1;
		bbox tex0_coords, tex1_coords;
		quad dest_coords;
		rgba color;
		float depth;
	};

	void render(const texture *tex, const sprite_info **sprites, size_t num_sprites);
	void render(const texture *tex0, const texture *tex1, const sprite_info **sprites, size_t num_sprites);

	rgba color_;
	mat3 matrix_;
	std::stack<mat3> matrix_stack_;
	std::vector<sprite_info> sprites_;

	gl_buffer verts_buffer_;
	gl_buffer indices_buffer_;

	gl_vertex_array vao_single_;
	gl_vertex_array vao_multi_;
};

}
