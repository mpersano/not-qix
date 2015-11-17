#include <algorithm>

#include <ggl/util.h>
#include <ggl/gl_buffer.h>
#include <ggl/texture.h>
#include <ggl/sprite_batch.h>

namespace ggl {

namespace {

const size_t INDICES_PER_SPRITE = 6;
const size_t VERTS_PER_SPRITE = 4;
const size_t MAX_SPRITES_PER_BATCH = 32;

struct gl_vertex_single
{
	GLfloat pos[2];
	GLfloat texcoord[2];
	GLfloat color[4];
};

struct gl_vertex_multi
{
	GLfloat pos[2];
	GLfloat texcoord0[2];
	GLfloat texcoord1[2];
	GLfloat color[4];
};

};

sprite_batch::sprite_batch()
: verts_buffer_ { GL_ARRAY_BUFFER }
, indices_buffer_ { GL_ELEMENT_ARRAY_BUFFER }
{
	// vertex buffer

	verts_buffer_.bind();
	verts_buffer_.buffer_data(MAX_SPRITES_PER_BATCH*VERTS_PER_SPRITE*sizeof(gl_vertex_multi), nullptr, GL_DYNAMIC_DRAW);
	verts_buffer_.unbind();

	// index buffer

	indices_buffer_.bind();
	indices_buffer_.buffer_data(MAX_SPRITES_PER_BATCH*INDICES_PER_SPRITE*sizeof(GLushort), nullptr, GL_STATIC_DRAW);

	auto index_ptr = reinterpret_cast<GLushort *>(indices_buffer_.map(GL_WRITE_ONLY));

	for (int i = 0; i < MAX_SPRITES_PER_BATCH; i++) {
		*index_ptr++ = i*4;
		*index_ptr++ = i*4 + 1;
		*index_ptr++ = i*4 + 2;

		*index_ptr++ = i*4 + 2;
		*index_ptr++ = i*4 + 3;
		*index_ptr++ = i*4;
	}

	indices_buffer_.unmap();
	indices_buffer_.unbind();

	// vao for single texture sprites

	vao_single_.bind();

	verts_buffer_.bind();

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, sizeof(gl_vertex_single), reinterpret_cast<GLvoid *>(offsetof(gl_vertex_single, pos)));

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(gl_vertex_single), reinterpret_cast<GLvoid *>(offsetof(gl_vertex_single, texcoord)));

	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_FLOAT, sizeof(gl_vertex_single), reinterpret_cast<GLvoid *>(offsetof(gl_vertex_single, color)));

	// vao for multi texture sprites

	verts_buffer_.unbind();
	vao_multi_.bind();

	verts_buffer_.bind(); // XXX need this?

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, sizeof(gl_vertex_multi), reinterpret_cast<GLvoid *>(offsetof(gl_vertex_multi, pos)));

	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(gl_vertex_multi), reinterpret_cast<GLvoid *>(offsetof(gl_vertex_multi, texcoord0)));

	glClientActiveTexture(GL_TEXTURE1);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(gl_vertex_multi), reinterpret_cast<GLvoid *>(offsetof(gl_vertex_multi, texcoord1)));

	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_FLOAT, sizeof(gl_vertex_multi), reinterpret_cast<GLvoid *>(offsetof(gl_vertex_multi, color)));

	glClientActiveTexture(GL_TEXTURE0);

	verts_buffer_.unbind();
	gl_vertex_array::unbind();
}

void
sprite_batch::begin()
{
	sprites_.clear();

	matrix_ = mat3::identity();
	color_ = white;
	matrix_stack_ = std::stack<mat3>();
}

void
sprite_batch::set_color(const rgba& color)
{
	color_ = color;
}

void
sprite_batch::translate(float x, float y)
{
	matrix_ *= mat3::translation(x, y);
}

void
sprite_batch::translate(const vec2f& pos)
{
	matrix_ *= mat3::translation(pos);
}

void
sprite_batch::scale(float s)
{
	matrix_ *= mat3::scale(s, s);
}

void
sprite_batch::scale(float sx, float sy)
{
	matrix_ *= mat3::scale(sx, sy);
}

void
sprite_batch::rotate(float a)
{
	matrix_ *= mat3::rotation(a);
}

void
sprite_batch::push_matrix()
{
	matrix_stack_.push(matrix_);
}

void
sprite_batch::pop_matrix()
{
	// XXX check if empty
	matrix_ = matrix_stack_.top();
	matrix_stack_.pop();
}

void
sprite_batch::draw(const texture *tex, const bbox& tex_coords, const bbox& dest_coords, float depth)
{
	const float x0 = dest_coords.min.x;
	const float x1 = dest_coords.max.x;

	const float y0 = dest_coords.min.y;
	const float y1 = dest_coords.max.y;

	draw(tex, tex_coords, { { x0, y0 }, { x0, y1 }, { x1, y1 }, { x1, y0 } }, depth);
}

void
sprite_batch::draw(const texture *tex, const bbox& tex_coords, const quad& dest_coords, float depth)
{
	draw(tex, nullptr, tex_coords, bbox(), dest_coords, depth);
}

void
sprite_batch::draw(const texture *tex0, const texture *tex1, const bbox& tex0_coords, const bbox& tex1_coords, const bbox& dest_coords, float depth)
{
	const float x0 = dest_coords.min.x;
	const float x1 = dest_coords.max.x;

	const float y0 = dest_coords.min.y;
	const float y1 = dest_coords.max.y;

	draw(tex0, tex1, tex0_coords, tex1_coords, { { x0, y0 }, { x0, y1 }, { x1, y1 }, { x1, y0 } }, depth);
}

void
sprite_batch::draw(const texture *tex0, const texture *tex1, const bbox& tex0_coords, const bbox& tex1_coords, const quad& dest_coords, float depth)
{
	sprites_.push_back(
		{ tex0,
		  tex1,
		  tex0_coords,
		  tex1_coords,
		  { matrix_*dest_coords.p0, matrix_*dest_coords.p1, matrix_*dest_coords.p2, matrix_*dest_coords.p3 },
		  color_,
		  depth });
}

void
sprite_batch::end()
{
	if (sprites_.empty())
		return;

	std::vector<const sprite_info *> sorted_sprites(sprites_.size());

	std::transform(
		std::begin(sprites_),
		std::end(sprites_),
		std::begin(sorted_sprites),
		[](const sprite_info& v) { return &v; });

	std::stable_sort(
		std::begin(sorted_sprites),
		std::end(sorted_sprites),
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

	enable_alpha_blend _;
	glColor4f(1, 1, 1, 1); // XXX for now

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);

	const texture *batch_tex0 = nullptr;
	const texture *batch_tex1 = nullptr;

	size_t batch_start = 0;

	auto do_render = [&](size_t end)
		{
			const auto start = &sorted_sprites[batch_start];
			const auto num_sprites = end - batch_start;

			if (batch_tex1 == nullptr)
				render(batch_tex0, start, num_sprites);
			else
				render(batch_tex0, batch_tex1, start, num_sprites);
		};

	size_t num_sprites = sorted_sprites.size();

	for (size_t i = 0; i < num_sprites; i++) {
		auto sp = sorted_sprites[i];

		if (sp->tex0 != batch_tex0 || sp->tex1 != batch_tex1 || i - batch_start == MAX_SPRITES_PER_BATCH) {
			if (i > 0)
				do_render(i);

			batch_tex0 = sp->tex0;
			batch_tex1 = sp->tex1;
			batch_start = i;
		}
	}

	do_render(num_sprites);

	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
}

void
sprite_batch::render(const texture *tex0, const texture *tex1, const sprite_info **sprites, size_t num_sprites)
{
	glActiveTexture(GL_TEXTURE0);
	tex0->bind();

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	tex1->bind();

	verts_buffer_.bind();

	auto vert_ptr = reinterpret_cast<gl_vertex_multi *>(verts_buffer_.map(GL_WRITE_ONLY));

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

	verts_buffer_.unmap();

	vao_multi_.bind();

	indices_buffer_.bind();
	glDrawElements(GL_TRIANGLES, num_sprites*INDICES_PER_SPRITE, GL_UNSIGNED_SHORT, 0);
	indices_buffer_.unbind();

	gl_vertex_array::unbind();

	verts_buffer_.unbind(); // XXX need this?
}

void
sprite_batch::render(const texture *tex, const sprite_info **sprites, size_t num_sprites)
{
	glActiveTexture(GL_TEXTURE0);
	tex->bind();

	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);

	verts_buffer_.bind();

	auto vert_ptr = reinterpret_cast<gl_vertex_single *>(verts_buffer_.map(GL_WRITE_ONLY));

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

	verts_buffer_.unmap();

	vao_single_.bind();

	indices_buffer_.bind();
	glDrawElements(GL_TRIANGLES, num_sprites*INDICES_PER_SPRITE, GL_UNSIGNED_SHORT, 0);
	indices_buffer_.unbind();

	gl_vertex_array::unbind();

	verts_buffer_.unbind(); // XXX need this?
}

}
