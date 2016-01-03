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
#include <ggl/mesh.h>
#include <ggl/gl_vertex_array.h>
#include <ggl/gl_buffer.h>
#include <ggl/program.h>
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
	bbox get_viewport() const;

	void begin();

	void translate(const vec2f& p);
	void scale(const vec2f& s);

	void rotate(float a);

	void push_matrix();
	void pop_matrix();

	mat3 get_matrix() const;

	void set_color(const rgba& color);

	void enqueue(const quad& dest_coords, float depth);
	void enqueue(const texture *tex0, const bbox& tex0_coords, const quad& dest_coords, float depth);
	void enqueue(const texture *tex0, const texture *tex1, const bbox& tex0_coords, const bbox& tex1_coords, const quad& dest_coords, float depth);
	void enqueue(const mesh *m, const mat4& mat, float depth);

	void end();

	const GLfloat *get_proj_modelview() const
	{ return &ortho_proj_[0]; }

private:
	void init_buffers();
	void init_vaos();

	void init_perspective_proj();
	void init_ortho_proj();

	struct primitive_info
	{
		primitive_info() { } // har har
		enum { QUAD, MESH } type;
		float depth;
		union {
			struct {
				const texture *tex0, *tex1;
				bbox tex0_coords, tex1_coords;
				quad dest_coords;
				rgba color;
			} quad_info;
			struct {
				const mesh *m;
				mat4 mat;
			} mesh_info;
		};
	};

	// untextured quads
	void render_quads(const primitive_info *const *sprites, size_t num_sprites);

	// textured quads
	void render_quads(const texture *tex, const primitive_info *const *sprites, size_t num_sprites);

	// 2-textured quads
	void render_quads(const texture *tex0, const texture *tex1, const primitive_info *const *sprites, size_t num_sprites);

	// meshes
	void render_meshes(const primitive_info *const *meshes, size_t num_meshes);

	rgba color_;
	mat3 matrix_;
	std::stack<mat3> matrix_stack_;

	static const int SPRITE_QUEUE_CAPACITY = 1024;

	int sprite_queue_size_;
	primitive_info sprite_queue_[SPRITE_QUEUE_CAPACITY];

	gl_buffer vert_buffer_;
	gl_buffer index_buffer_;

	gl_vertex_array vao_color_;
	gl_vertex_array vao_single_;
	gl_vertex_array vao_multi_;

	const program *prog_color_;
	const program *prog_single_;
	const program *prog_multi_;
	const program *prog_mesh_;
	const program *prog_mesh_outline_;

	bbox viewport_;
	std::array<GLfloat, 16> ortho_proj_;
	std::array<GLfloat, 16> perspective_proj_;
} *g_renderer;

renderer::renderer()
: vert_buffer_ { GL_ARRAY_BUFFER }
, index_buffer_ { GL_ELEMENT_ARRAY_BUFFER }
, prog_color_ { res::get_program("color") }
, prog_single_ { res::get_program("texture-color") }
, prog_multi_ { res::get_program("bitexture-color") }
, prog_mesh_ { res::get_program("mesh") }
, prog_mesh_outline_ { res::get_program("mesh-outline") }
{
	init_buffers();
	init_vaos();

	prog_single_->use();
	prog_single_->set_uniform_i("tex", 0); // texunit 0

	prog_multi_->use();
	prog_multi_->set_uniform_i("tex0", 0); // texunit 0
	prog_multi_->set_uniform_i("tex1", 1); // texunit 0
}

void
renderer::set_viewport(const bbox& viewport)
{
	viewport_ = viewport;

	init_ortho_proj();
	init_perspective_proj();
}

void
renderer::init_ortho_proj()
{
	const float Z_NEAR = -1.f;
	const float Z_FAR = 1.f;

	const float a = 2.f/(viewport_.max.x - viewport_.min.x);
	const float b = 2.f/(viewport_.max.y - viewport_.min.y);
	const float c = -2.f/(Z_FAR - Z_NEAR);

	const float tx = -(viewport_.max.x + viewport_.min.x)/(viewport_.max.x - viewport_.min.x);
	const float ty = -(viewport_.max.y + viewport_.min.y)/(viewport_.max.y - viewport_.min.y);
	const float tz = -(Z_FAR + Z_NEAR)/(Z_FAR - Z_NEAR);

	ortho_proj_ = { a, 0, 0, tx,
			0, b, 0, ty,
			0, 0, c, tz,
			0, 0, 0, 1 };

	prog_color_->use();
	prog_color_->set_uniform_mat4("proj_modelview", &ortho_proj_[0]);

	prog_single_->use();
	prog_single_->set_uniform_mat4("proj_modelview", &ortho_proj_[0]);

	prog_multi_->use();
	prog_multi_->set_uniform_mat4("proj_modelview", &ortho_proj_[0]);
}

void
renderer::init_perspective_proj()
{
	const auto viewport_width = viewport_.max.x - viewport_.min.x;
	const auto viewport_height = viewport_.max.y - viewport_.min.y;

	const float FOV = 30.f;
	const float Z_NEAR = 1.f;
	const float Z_FAR = 1000.f;

	const float aspect = static_cast<float>(viewport_width)/viewport_height;

	const float fovy_rad = FOV*M_PI/180.;

	const float f = 1.f/tanf(.5f*fovy_rad);

	perspective_proj_ = { f/aspect, 0, 0, 0,
			      0, f, 0, 0,
			      0, 1, (Z_FAR + Z_NEAR)/(Z_NEAR - Z_FAR), -1,
			      0, 0, (2.*Z_FAR*Z_NEAR)/(Z_NEAR - Z_FAR), 0 };

	prog_mesh_->use();
	prog_mesh_->set_uniform_mat4("proj_matrix", &perspective_proj_[0]);

	prog_mesh_outline_->use();
	prog_mesh_outline_->set_uniform_mat4("proj_matrix", &perspective_proj_[0]);
}

bbox
renderer::get_viewport() const
{
	return viewport_;
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

mat3
renderer::get_matrix() const
{
	return matrix_;
}

void
renderer::enqueue(const quad& dest_coords, float depth)
{
	assert(sprite_queue_size_ < SPRITE_QUEUE_CAPACITY);

	auto p = &sprite_queue_[sprite_queue_size_++];

	p->type = primitive_info::QUAD;
	p->depth = depth;

	auto& q = p->quad_info;
	q.tex0 = nullptr;
	q.tex1 = nullptr;
	q.dest_coords = { matrix_*dest_coords.p0, matrix_*dest_coords.p1, matrix_*dest_coords.p2, matrix_*dest_coords.p3 };
	q.color = color_;
}

void
renderer::enqueue(const texture *tex0, const bbox& tex0_coords, const quad& dest_coords, float depth)
{
	assert(sprite_queue_size_ < SPRITE_QUEUE_CAPACITY);

	auto p = &sprite_queue_[sprite_queue_size_++];

	p->type = primitive_info::QUAD;
	p->depth = depth;

	auto& q = p->quad_info;
	q.tex0 = tex0;
	q.tex1 = nullptr;
	q.tex0_coords = tex0_coords;
	q.dest_coords = { matrix_*dest_coords.p0, matrix_*dest_coords.p1, matrix_*dest_coords.p2, matrix_*dest_coords.p3 };
	q.color = color_;
}

void
renderer::enqueue(const texture *tex0, const texture *tex1, const bbox& tex0_coords, const bbox& tex1_coords, const quad& dest_coords, float depth)
{
	assert(sprite_queue_size_ < SPRITE_QUEUE_CAPACITY);

	auto p = &sprite_queue_[sprite_queue_size_++];

	p->type = primitive_info::QUAD;
	p->depth = depth;

	auto& q = p->quad_info;
	q.tex0 = tex0;
	q.tex1 = tex1;
	q.tex0_coords = tex0_coords;
	q.tex1_coords = tex1_coords;
	q.dest_coords = { matrix_*dest_coords.p0, matrix_*dest_coords.p1, matrix_*dest_coords.p2, matrix_*dest_coords.p3 };
	q.color = color_;
}

void
renderer::enqueue(const mesh *m, const mat4& mat, float depth)
{
	assert(sprite_queue_size_ < SPRITE_QUEUE_CAPACITY);

	auto p = &sprite_queue_[sprite_queue_size_++];

	p->type = primitive_info::MESH;
	p->depth = depth;

	auto& mi = p->mesh_info;

	mi.m = m;

	const float PLANE_Z = -50.f;

	const vec2f po = vec2f { matrix_.m02, matrix_.m12 } - viewport_.min;

	const auto viewport_width = viewport_.max.x - viewport_.min.x;
	const auto viewport_height = viewport_.max.y - viewport_.min.y;

	const float xl = 2.f*(po.x/viewport_width - .5f);
	const float yl = 2.f*(po.y/viewport_height - .5f);

	// XXX why do we need to double here?!!!
	const float x = 2.f*xl*PLANE_Z/-perspective_proj_[0];
	const float y = 2.f*yl*PLANE_Z/-perspective_proj_[5];

	mi.mat = mat4 { matrix_.m00, matrix_.m01, 0, x,
		        matrix_.m10, matrix_.m11, 0, y,
		                  0,           0, 1, PLANE_Z }*mat;
}

void
renderer::end()
{
	if (!sprite_queue_size_)
		return;

	// sort sprites

	static const primitive_info *sorted_sprites[SPRITE_QUEUE_CAPACITY];

	for (size_t i = 0; i < sprite_queue_size_; i++)
		sorted_sprites[i] = &sprite_queue_[i];

	std::stable_sort(
		&sorted_sprites[0],
		&sorted_sprites[sprite_queue_size_],
		[](const primitive_info *a, const primitive_info *b)
		{
			if (a->depth < b->depth) {
				return true;
			} else if (a->depth > b->depth) {
				return false;
			} else if (a->type < b->type) {
				return true;
			} else if (a->type > b->type) {
				return false;
			} else {
				if (a->type == primitive_info::QUAD) {
					if (a->quad_info.tex0 < b->quad_info.tex0)
						return true;
					else if (a->quad_info.tex0 > b->quad_info.tex0)
						return false;
					else
						return a->quad_info.tex1 < b->quad_info.tex1;
				} else {
					return a->mesh_info.m < b->mesh_info.m;
				}
			}
		});

	// do the dance, do the dance

	size_t batch_start = 0;

	int batch_primitive_type = -1;
	const texture *batch_tex0 = nullptr;
	const texture *batch_tex1 = nullptr;

	auto do_render = [&](size_t end)
		{
			const auto start = &sorted_sprites[batch_start];
			const auto num_sprites = end - batch_start;

			if (!num_sprites)
				return;

			if (batch_primitive_type == primitive_info::MESH) {
				render_meshes(start, num_sprites);
			} else {
				if (batch_tex0 == nullptr) {
					assert(batch_tex1 == nullptr);
					render_quads(start, num_sprites);
				} else if (batch_tex1 == nullptr) {
					render_quads(batch_tex0, start, num_sprites);
				} else {
					render_quads(batch_tex0, batch_tex1, start, num_sprites);
				}
			}
		};

	{
	enable_alpha_blend _;

	vert_buffer_.bind();

	for (size_t i = 0; i < sprite_queue_size_; i++) {
		auto sp = sorted_sprites[i];

		if (sp->type != batch_primitive_type ||
		    (sp->type == primitive_info::QUAD && (sp->quad_info.tex0 != batch_tex0 || sp->quad_info.tex1 != batch_tex1))) {
			do_render(i);

			batch_primitive_type = sp->type;

			if (sp->type == primitive_info::QUAD) {
				batch_tex0 = sp->quad_info.tex0;
				batch_tex1 = sp->quad_info.tex1;
			}

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
renderer::render_quads(const texture *tex0, const texture *tex1, const primitive_info *const *sprites, size_t num_sprites)
{
	gl_check(glActiveTexture(GL_TEXTURE0));
	tex0->bind();

	gl_check(glActiveTexture(GL_TEXTURE1));
	tex1->bind();

	prog_multi_->use();

	auto vert_ptr = reinterpret_cast<gl_vertex_multi *>(vert_buffer_.map_range(0, num_sprites*VERTS_PER_SPRITE*sizeof(gl_vertex_multi), GL_MAP_WRITE_BIT));

	for (size_t i = 0; i < num_sprites; i++) {
		auto sp = sprites[i];

		assert(sp->type == primitive_info::QUAD);

		// XXX could move this shit to constructor and use placement new

		const auto& dest_coords = sp->quad_info.dest_coords;

		const GLfloat x0 = dest_coords.p0.x;
		const GLfloat y0 = dest_coords.p0.y;

		const GLfloat x1 = dest_coords.p1.x;
		const GLfloat y1 = dest_coords.p1.y;

		const GLfloat x2 = dest_coords.p2.x;
		const GLfloat y2 = dest_coords.p2.y;

		const GLfloat x3 = dest_coords.p3.x;
		const GLfloat y3 = dest_coords.p3.y;

		const auto& tex0_coords = sp->quad_info.tex0_coords;

		const GLfloat u00 = tex0_coords.min.x;
		const GLfloat u10 = tex0_coords.max.x;

		const GLfloat v00 = tex0_coords.min.y;
		const GLfloat v10 = tex0_coords.max.y;

		const auto& tex1_coords = sp->quad_info.tex1_coords;

		const GLfloat u01 = tex1_coords.min.x;
		const GLfloat u11 = tex1_coords.max.x;

		const GLfloat v01 = tex1_coords.min.y;
		const GLfloat v11 = tex1_coords.max.y;

		const auto& color = sp->quad_info.color;

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
renderer::render_quads(const texture *tex, const primitive_info *const *sprites, size_t num_sprites)
{
	gl_check(glActiveTexture(GL_TEXTURE0));
	tex->bind();

	prog_single_->use();

	auto vert_ptr = reinterpret_cast<gl_vertex_single *>(vert_buffer_.map_range(0, num_sprites*VERTS_PER_SPRITE*sizeof(gl_vertex_single), GL_MAP_WRITE_BIT));

	for (size_t i = 0; i < num_sprites; i++) {
		auto sp = sprites[i];

		assert(sp->type == primitive_info::QUAD);

		const auto& dest_coords = sp->quad_info.dest_coords;

		const GLfloat x0 = dest_coords.p0.x;
		const GLfloat y0 = dest_coords.p0.y;

		const GLfloat x1 = dest_coords.p1.x;
		const GLfloat y1 = dest_coords.p1.y;

		const GLfloat x2 = dest_coords.p2.x;
		const GLfloat y2 = dest_coords.p2.y;

		const GLfloat x3 = dest_coords.p3.x;
		const GLfloat y3 = dest_coords.p3.y;

		const auto& tex_coords = sp->quad_info.tex0_coords;

		const GLfloat u0 = tex_coords.min.x;
		const GLfloat u1 = tex_coords.max.x;

		const GLfloat v0 = tex_coords.min.y;
		const GLfloat v1 = tex_coords.max.y;

		const auto& color = sp->quad_info.color;

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
renderer::render_quads(const primitive_info *const *sprites, size_t num_sprites)
{
	prog_color_->use();

	auto vert_ptr = reinterpret_cast<gl_vertex_color *>(vert_buffer_.map_range(0, num_sprites*VERTS_PER_SPRITE*sizeof(gl_vertex_color), GL_MAP_WRITE_BIT));

	for (size_t i = 0; i < num_sprites; i++) {
		auto sp = sprites[i];

		assert(sp->type == primitive_info::QUAD);

		const auto& dest_coords = sp->quad_info.dest_coords;

		const GLfloat x0 = dest_coords.p0.x;
		const GLfloat y0 = dest_coords.p0.y;

		const GLfloat x1 = dest_coords.p1.x;
		const GLfloat y1 = dest_coords.p1.y;

		const GLfloat x2 = dest_coords.p2.x;
		const GLfloat y2 = dest_coords.p2.y;

		const GLfloat x3 = dest_coords.p3.x;
		const GLfloat y3 = dest_coords.p3.y;

		const auto& color = sp->quad_info.color;

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
renderer::render_meshes(const primitive_info *const *meshes, size_t num_meshes)
{
	gl_check(glEnable(GL_CULL_FACE));

	// draw outlines

	prog_mesh_outline_->use();
	prog_mesh_outline_->set_uniform_f("offs", .25);
	prog_mesh_outline_->set_uniform_f("color", 0, 0, 0, 1);

	gl_check(glFrontFace(GL_CW));

	for (size_t i = 0; i < num_meshes; i++) {
		auto sp = meshes[i];
		assert(sp->type == primitive_info::MESH);

		auto& m = sp->mesh_info;
		prog_mesh_outline_->set_uniform_mat4("modelview_matrix", m.mat);

		m.m->draw();
	}

	// draw meshes

	prog_mesh_->use();

	gl_check(glFrontFace(GL_CCW));

	for (size_t i = 0; i < num_meshes; i++) {
		auto sp = meshes[i];
		assert(sp->type == primitive_info::MESH);

		auto& m = sp->mesh_info;
		prog_mesh_->set_uniform_mat4("modelview_matrix", m.mat);

		m.m->draw();
	}

	gl_check(glDisable(GL_CULL_FACE));
}

void
init()
{
	g_renderer = new renderer;
}

void
shutdown()
{
	delete g_renderer;
}

void
set_viewport(const bbox& viewport)
{
	g_renderer->set_viewport(viewport);
}

bbox
get_viewport()
{
	return g_renderer->get_viewport();
}

mat3
get_matrix()
{
	return g_renderer->get_matrix();
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
draw(const mesh *m, const mat4& mat, float depth)
{
	g_renderer->enqueue(m, mat, depth);
}

void
end()
{
	g_renderer->end();
}

const GLfloat *
get_proj_modelview()
{
	return g_renderer->get_proj_modelview();
}

} }
