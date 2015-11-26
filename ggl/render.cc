#include <algorithm>
#include <vector>
#include <stack>
#include <memory>
#include <cassert>

#include <ggl/log.h>
#include <ggl/noncopyable.h>
#include <ggl/resources.h>
#include <ggl/sprite.h>
#include <ggl/vec3.h>
#include <ggl/mat3.h>
#include <ggl/rgba.h>
#include <ggl/gl_vertex_array.h>
#include <ggl/gl_buffer.h>
#include <ggl/gl_program.h>
#include <ggl/util.h>
#include <ggl/gl_buffer.h>
#include <ggl/texture.h>
#include <ggl/gl_check.h>
#include <ggl/render.h>

namespace ggl { namespace render {

namespace {

const size_t INDICES_PER_SPRITE = 6;
const size_t VERTS_PER_SPRITE = 4;

struct gl_vertex_color {
	GLfloat position[2];
	GLfloat color[4];
};

struct gl_vertex_single {
	GLfloat position[2];
	GLfloat texcoord[2];
	GLfloat color[4];
};

struct gl_vertex_multi {
	GLfloat position[2];
	GLfloat texcoord0[2];
	GLfloat texcoord1[2];
	GLfloat color[4];
};

} // (anonymous namespace)

class renderer : private noncopyable
{
public:
	renderer();

	void set_viewport(const bbox& viewport);

	void begin();

	void translate(const vec2f& p);
	void scale(const vec2f& s);

	void rotate(float a);

	void push_matrix();
	void pop_matrix();

	void set_color(const rgba& color);

	void enqueue(const quad& dest_coords, float depth);
	void enqueue(const texture *tex0, const bbox& tex0_coords, const quad& dest_coords, float depth);
	void enqueue(const texture *tex0, const texture *tex1, const bbox& tex0_coords, const bbox& tex1_coords, const quad& dest_coords, float depth);

	void end();

	mat4 get_proj_modelview() const
	{ return proj_modelview_; }

private:
	void init_buffers();
	void init_vaos();

	struct sprite_info
	{
		const texture *tex0, *tex1;
		bbox tex0_coords, tex1_coords;
		quad dest_coords;
		rgba color;
		float depth;
	};

	// untextured quads
	void render(const sprite_info *const *sprites, size_t num_sprites);

	// textured quads
	void render(const texture *tex, const sprite_info *const *sprites, size_t num_sprites);

	// 2-textured quads
	void render(const texture *tex0, const texture *tex1, const sprite_info *const *sprites, size_t num_sprites);

	rgba color_;
	mat3 matrix_;
	std::stack<mat3> matrix_stack_;

	static const int SPRITE_QUEUE_CAPACITY = 1024;

	int sprite_queue_size_;
	sprite_info sprite_queue_[SPRITE_QUEUE_CAPACITY];

	gl_buffer vert_buffer_;
	gl_buffer index_buffer_;

	gl_vertex_array vao_color_;
	gl_vertex_array vao_single_;
	gl_vertex_array vao_multi_;

	const gl_program *program_color_;
	const gl_program *program_single_;
	const gl_program *program_multi_;

	mat4 proj_modelview_;
} *g_renderer;

renderer::renderer()
: vert_buffer_ { GL_ARRAY_BUFFER }
, index_buffer_ { GL_ELEMENT_ARRAY_BUFFER }
, proj_modelview_ { mat4::identity() }
, program_color_ { res::get_program("color") }
, program_single_ { res::get_program("texture-color") }
, program_multi_ { res::get_program("bitexture-color") }
{

	init_buffers();
	init_vaos();
}

void
renderer::set_viewport(const bbox& viewport)
{
	const float NEAR = -1.f;
	const float FAR = 1.f;

	const float a = 2.f/(viewport.max.x - viewport.min.x);
	const float b = 2.f/(viewport.max.y - viewport.min.y);
	const float c = -2.f/(FAR - NEAR);

	const float tx = -(viewport.max.x + viewport.min.x)/(viewport.max.x - viewport.min.x);
	const float ty = -(viewport.max.y + viewport.min.y)/(viewport.max.y - viewport.min.y);
	const float tz = -(FAR + NEAR)/(FAR - NEAR);

	proj_modelview_ = mat4(
				a, 0, 0, tx,
				0, b, 0, ty,
				0, 0, c, tz);
}

void
renderer::init_buffers()
{
	// vertex buffer

	vert_buffer_.bind();
	vert_buffer_.buffer_data(SPRITE_QUEUE_CAPACITY*VERTS_PER_SPRITE*sizeof(gl_vertex_multi), nullptr, GL_DYNAMIC_DRAW);
	vert_buffer_.unbind();

	// index buffer

	GLsizei index_buffer_size = SPRITE_QUEUE_CAPACITY*INDICES_PER_SPRITE*sizeof(GLushort);

	index_buffer_.bind();
	index_buffer_.buffer_data(index_buffer_size, nullptr, GL_DYNAMIC_DRAW);

	auto index_ptr = reinterpret_cast<GLushort *>(index_buffer_.map_range(0, index_buffer_size, GL_MAP_WRITE_BIT));

	for (int i = 0; i < SPRITE_QUEUE_CAPACITY; i++) {
		*index_ptr++ = i*4;
		*index_ptr++ = i*4 + 1;
		*index_ptr++ = i*4 + 2;

		*index_ptr++ = i*4 + 2;
		*index_ptr++ = i*4 + 3;
		*index_ptr++ = i*4;
	}

	index_buffer_.unmap();
	index_buffer_.unbind();
}

void
renderer::init_vaos()
{
#define ENABLE_ATTRIB(location, size, vt, field) \
	gl_check(glVertexAttribPointer(location, size, GL_FLOAT, GL_FALSE, sizeof(vt), reinterpret_cast<const GLvoid *>(offsetof(vt, field)))); \
	gl_check(glEnableVertexAttribArray(location));

	vao_color_.bind();
	vert_buffer_.bind();
	ENABLE_ATTRIB(0, 2, gl_vertex_color, position)
	ENABLE_ATTRIB(1, 4, gl_vertex_color, color)
	vert_buffer_.unbind();

	vao_single_.bind();
	vert_buffer_.bind();
	ENABLE_ATTRIB(0, 2, gl_vertex_single, position)
	ENABLE_ATTRIB(1, 2, gl_vertex_single, texcoord)
	ENABLE_ATTRIB(2, 4, gl_vertex_single, color)
	vert_buffer_.unbind();

	vao_multi_.bind();
	vert_buffer_.bind();
	ENABLE_ATTRIB(0, 2, gl_vertex_multi, position)
	ENABLE_ATTRIB(1, 2, gl_vertex_multi, texcoord0)
	ENABLE_ATTRIB(2, 2, gl_vertex_multi, texcoord1)
	ENABLE_ATTRIB(3, 4, gl_vertex_multi, color)
	vert_buffer_.unbind();

#undef ENABLE_ATTRIB

	gl_vertex_array::unbind();
}

void
renderer::begin()
{
	sprite_queue_size_ = 0;

	matrix_ = mat3::identity();
	color_ = white;
	matrix_stack_ = std::stack<mat3>();
}

void
renderer::set_color(const rgba& color)
{
	color_ = color;
}

void
renderer::translate(const vec2f& p)
{
	matrix_ *= mat3::translation(p);
}

void
renderer::scale(const vec2f& s)
{
	matrix_ *= mat3::scale(s);
}

void
renderer::rotate(float a)
{
	matrix_ *= mat3::rotation(a);
}

void
renderer::push_matrix()
{
	matrix_stack_.push(matrix_);
}

void
renderer::pop_matrix()
{
	assert(!matrix_stack_.empty());
	matrix_ = matrix_stack_.top();
	matrix_stack_.pop();
}


void
renderer::enqueue(const quad& dest_coords, float depth)
{
	assert(sprite_queue_size_ < SPRITE_QUEUE_CAPACITY);

	auto p = &sprite_queue_[sprite_queue_size_++];

	p->tex0 = nullptr;
	p->tex1 = nullptr;
	p->dest_coords = { matrix_*dest_coords.p0, matrix_*dest_coords.p1, matrix_*dest_coords.p2, matrix_*dest_coords.p3 };
	p->color = color_;
	p->depth = depth;
}

void
renderer::enqueue(const texture *tex0, const bbox& tex0_coords, const quad& dest_coords, float depth)
{
	assert(sprite_queue_size_ < SPRITE_QUEUE_CAPACITY);

	auto p = &sprite_queue_[sprite_queue_size_++];

	p->tex0 = tex0;
	p->tex1 = nullptr;
	p->tex0_coords = tex0_coords;
	p->dest_coords = { matrix_*dest_coords.p0, matrix_*dest_coords.p1, matrix_*dest_coords.p2, matrix_*dest_coords.p3 };
	p->color = color_;
	p->depth = depth;
}

void
renderer::enqueue(const texture *tex0, const texture *tex1, const bbox& tex0_coords, const bbox& tex1_coords, const quad& dest_coords, float depth)
{
	assert(sprite_queue_size_ < SPRITE_QUEUE_CAPACITY);

	auto p = &sprite_queue_[sprite_queue_size_++];

	p->tex0 = tex0;
	p->tex1 = tex1;
	p->tex0_coords = tex0_coords;
	p->tex1_coords = tex1_coords;
	p->dest_coords = { matrix_*dest_coords.p0, matrix_*dest_coords.p1, matrix_*dest_coords.p2, matrix_*dest_coords.p3 };
	p->color = color_;
	p->depth = depth;
}

void
renderer::end()
{
	if (!sprite_queue_size_)
		return;

	// sort sprites

	static const sprite_info *sorted_sprites[SPRITE_QUEUE_CAPACITY];

	for (size_t i = 0; i < sprite_queue_size_; i++)
		sorted_sprites[i] = &sprite_queue_[i];

	std::stable_sort(
		&sorted_sprites[0],
		&sorted_sprites[sprite_queue_size_],
		[](const sprite_info *a, const sprite_info *b)
		{
			if (a->depth < b->depth)
				return true;
			else if (a->depth > b->depth)
				return false;
			else if (a->tex0 < b->tex0)
				return true;
			else if (a->tex0 > b->tex0)
				return false;
			else
				return a->tex1 < b->tex1;
		});

	// initialize program uniforms

	program_color_->use();
	program_color_->set_uniform_mat4("proj_modelview", proj_modelview_);

	program_single_->use();
	program_single_->set_uniform_i("tex", 0); // texunit 0
	program_single_->set_uniform_mat4("proj_modelview", proj_modelview_);

	program_multi_->use();
	program_multi_->set_uniform_i("tex0", 0); // texunit 0
	program_multi_->set_uniform_i("tex1", 1); // texunit 0
	program_multi_->set_uniform_mat4("proj_modelview", proj_modelview_);

	// do the dance, do the dance

	size_t batch_start = 0;
	const texture *batch_tex0 = sorted_sprites[0]->tex0;
	const texture *batch_tex1 = sorted_sprites[0]->tex1;

	auto do_render = [&](size_t end)
		{
			const auto start = &sorted_sprites[batch_start];
			const auto num_sprites = end - batch_start;

			if (batch_tex0 == nullptr) {
				assert(batch_tex1 == nullptr);
				render(start, num_sprites);
			} else if (batch_tex1 == nullptr) {
				render(batch_tex0, start, num_sprites);
			} else {
				render(batch_tex0, batch_tex1, start, num_sprites);
			}
		};

	{
	enable_alpha_blend _;

	vert_buffer_.bind();

	for (size_t i = 0; i < sprite_queue_size_; i++) {
		auto sp = sorted_sprites[i];

		if (sp->tex0 != batch_tex0 || sp->tex1 != batch_tex1) {
			assert(i > 0);
			do_render(i);

			batch_tex0 = sp->tex0;
			batch_tex1 = sp->tex1;
			batch_start = i;
		}
	}

	do_render(sprite_queue_size_);
	}

	// cleanup

	vert_buffer_.unbind();
	index_buffer_.unbind();

	gl_vertex_array::unbind();

	gl_check(glActiveTexture(GL_TEXTURE0));
}

void
renderer::render(const texture *tex0, const texture *tex1, const sprite_info *const *sprites, size_t num_sprites)
{
	gl_check(glActiveTexture(GL_TEXTURE0));
	tex0->bind();

	gl_check(glActiveTexture(GL_TEXTURE1));
	tex1->bind();

	program_multi_->use();

	auto vert_ptr = reinterpret_cast<gl_vertex_multi *>(vert_buffer_.map_range(0, num_sprites*VERTS_PER_SPRITE*sizeof(gl_vertex_multi), GL_MAP_WRITE_BIT));

	for (size_t i = 0; i < num_sprites; i++) {
		auto sp = sprites[i];

		// XXX could move this shit to constructor and use placement new

		const auto& dest_coords = sp->dest_coords;

		const GLfloat x0 = dest_coords.p0.x;
		const GLfloat y0 = dest_coords.p0.y;

		const GLfloat x1 = dest_coords.p1.x;
		const GLfloat y1 = dest_coords.p1.y;

		const GLfloat x2 = dest_coords.p2.x;
		const GLfloat y2 = dest_coords.p2.y;

		const GLfloat x3 = dest_coords.p3.x;
		const GLfloat y3 = dest_coords.p3.y;

		const auto& tex0_coords = sp->tex0_coords;

		const GLfloat u00 = tex0_coords.min.x;
		const GLfloat u10 = tex0_coords.max.x;

		const GLfloat v00 = tex0_coords.min.y;
		const GLfloat v10 = tex0_coords.max.y;

		const auto& tex1_coords = sp->tex1_coords;

		const GLfloat u01 = tex1_coords.min.x;
		const GLfloat u11 = tex1_coords.max.x;

		const GLfloat v01 = tex1_coords.min.y;
		const GLfloat v11 = tex1_coords.max.y;

		const auto& color = sp->color;

		const GLfloat r = color.r;
		const GLfloat g = color.g;
		const GLfloat b = color.b;
		const GLfloat a = color.a;

		*vert_ptr++ = { { x0, y0 }, { u00, v00 }, { u01, v01 }, { r, g, b, a } };
		*vert_ptr++ = { { x1, y1 }, { u00, v10 }, { u01, v11 }, { r, g, b, a } };
		*vert_ptr++ = { { x2, y2 }, { u10, v10 }, { u11, v11 }, { r, g, b, a } };
		*vert_ptr++ = { { x3, y3 }, { u10, v00 }, { u11, v01 }, { r, g, b, a } };
	}

	vert_buffer_.unmap();

	vao_multi_.bind();
	index_buffer_.bind();
	gl_check(glDrawElements(GL_TRIANGLES, num_sprites*INDICES_PER_SPRITE, GL_UNSIGNED_SHORT, 0));
}

void
renderer::render(const texture *tex, const sprite_info *const *sprites, size_t num_sprites)
{
	gl_check(glActiveTexture(GL_TEXTURE0));
	tex->bind();

	program_single_->use();

	auto vert_ptr = reinterpret_cast<gl_vertex_single *>(vert_buffer_.map_range(0, num_sprites*VERTS_PER_SPRITE*sizeof(gl_vertex_single), GL_MAP_WRITE_BIT));

	for (size_t i = 0; i < num_sprites; i++) {
		auto sp = sprites[i];

		const auto& dest_coords = sp->dest_coords;

		const GLfloat x0 = dest_coords.p0.x;
		const GLfloat y0 = dest_coords.p0.y;

		const GLfloat x1 = dest_coords.p1.x;
		const GLfloat y1 = dest_coords.p1.y;

		const GLfloat x2 = dest_coords.p2.x;
		const GLfloat y2 = dest_coords.p2.y;

		const GLfloat x3 = dest_coords.p3.x;
		const GLfloat y3 = dest_coords.p3.y;

		const auto& tex_coords = sp->tex0_coords;

		const GLfloat u0 = tex_coords.min.x;
		const GLfloat u1 = tex_coords.max.x;

		const GLfloat v0 = tex_coords.min.y;
		const GLfloat v1 = tex_coords.max.y;

		const auto& color = sp->color;

		const GLfloat r = color.r;
		const GLfloat g = color.g;
		const GLfloat b = color.b;
		const GLfloat a = color.a;

		*vert_ptr++ = { { x0, y0 }, { u0, v0 }, { r, g, b, a } };
		*vert_ptr++ = { { x1, y1 }, { u0, v1 }, { r, g, b, a } };
		*vert_ptr++ = { { x2, y2 }, { u1, v1 }, { r, g, b, a } };
		*vert_ptr++ = { { x3, y3 }, { u1, v0 }, { r, g, b, a } };
	}

	vert_buffer_.unmap();

	vao_single_.bind();
	index_buffer_.bind();
	gl_check(glDrawElements(GL_TRIANGLES, num_sprites*INDICES_PER_SPRITE, GL_UNSIGNED_SHORT, 0));
}

void
renderer::render(const sprite_info *const *sprites, size_t num_sprites)
{
	program_color_->use();

	auto vert_ptr = reinterpret_cast<gl_vertex_color *>(vert_buffer_.map_range(0, num_sprites*VERTS_PER_SPRITE*sizeof(gl_vertex_color), GL_MAP_WRITE_BIT));

	for (size_t i = 0; i < num_sprites; i++) {
		auto sp = sprites[i];

		const auto& dest_coords = sp->dest_coords;

		const GLfloat x0 = dest_coords.p0.x;
		const GLfloat y0 = dest_coords.p0.y;

		const GLfloat x1 = dest_coords.p1.x;
		const GLfloat y1 = dest_coords.p1.y;

		const GLfloat x2 = dest_coords.p2.x;
		const GLfloat y2 = dest_coords.p2.y;

		const GLfloat x3 = dest_coords.p3.x;
		const GLfloat y3 = dest_coords.p3.y;

		const auto& color = sp->color;

		const GLfloat r = color.r;
		const GLfloat g = color.g;
		const GLfloat b = color.b;
		const GLfloat a = color.a;

		*vert_ptr++ = { { x0, y0 }, { r, g, b, a } };
		*vert_ptr++ = { { x1, y1 }, { r, g, b, a } };
		*vert_ptr++ = { { x2, y2 }, { r, g, b, a } };
		*vert_ptr++ = { { x3, y3 }, { r, g, b, a } };
	}

	vert_buffer_.unmap();

	vao_color_.bind();
	index_buffer_.bind();
	gl_check(glDrawElements(GL_TRIANGLES, num_sprites*INDICES_PER_SPRITE, GL_UNSIGNED_SHORT, 0));
}

void
init(void)
{
	g_renderer = new renderer;
}

void
set_viewport(const bbox& viewport)
{
	g_renderer->set_viewport(viewport);
}

void
begin()
{
	g_renderer->begin();
}

void
translate(const vec2f& p)
{
	g_renderer->translate(p);
}

void
translate(float x, float y)
{
	g_renderer->translate({ x, y });
}

void
scale(float s)
{
	g_renderer->scale({ s, s });
}

void
scale(float sx, float sy)
{
	g_renderer->scale({ sx, sy });
}

void
scale(const vec2f& s)
{
	g_renderer->scale(s);
}

void
rotate(float a)
{
	g_renderer->rotate(a);
}

void
push_matrix()
{
	g_renderer->push_matrix();
}

void
pop_matrix()
{
	g_renderer->pop_matrix();
}

void
set_color(const rgba& color)
{
	g_renderer->set_color(color);
}

void
draw(const bbox& dest_coords, float depth)
{
	const float x0 = dest_coords.min.x;
	const float x1 = dest_coords.max.x;

	const float y0 = dest_coords.min.y;
	const float y1 = dest_coords.max.y;

	draw({ { x0, y0 }, { x0, y1 }, { x1, y1 }, { x1, y0 } }, depth);
}

void
draw(const quad& dest_coords, float depth)
{
	g_renderer->enqueue(dest_coords, depth);
}

void
draw(const texture *tex, const bbox& tex_coords, const bbox& dest_coords, float depth)
{
	const float x0 = dest_coords.min.x;
	const float x1 = dest_coords.max.x;

	const float y0 = dest_coords.min.y;
	const float y1 = dest_coords.max.y;

	draw(tex, tex_coords, { { x0, y0 }, { x0, y1 }, { x1, y1 }, { x1, y0 } }, depth);
}

void
draw(const texture *tex, const bbox& tex_coords, const quad& dest_coords, float depth)
{
	g_renderer->enqueue(tex, tex_coords, dest_coords, depth);
}

void
draw(const texture *tex0, const texture *tex1, const bbox& tex0_coords, const bbox& tex1_coords, const bbox& dest_coords, float depth)
{
	const float x0 = dest_coords.min.x;
	const float x1 = dest_coords.max.x;

	const float y0 = dest_coords.min.y;
	const float y1 = dest_coords.max.y;

	draw(tex0, tex1, tex0_coords, tex1_coords, { { x0, y0 }, { x0, y1 }, { x1, y1 }, { x1, y0 } }, depth);
}

void
draw(const texture *tex0, const texture *tex1, const bbox& tex_coords0, const bbox& tex_coords1, const quad& dest_coords, float depth)
{
	g_renderer->enqueue(tex0, tex1, tex_coords0, tex_coords1, dest_coords, depth);
}

void
end()
{
	g_renderer->end();
}

mat4
get_proj_modelview()
{
	return g_renderer->get_proj_modelview();
}

} }
