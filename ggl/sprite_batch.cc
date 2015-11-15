#include <algorithm>

#include <ggl/util.h>
#include <ggl/buffer_object.h>
#include <ggl/texture.h>
#include <ggl/sprite_batch.h>

namespace ggl {

namespace {

const size_t INDICES_PER_SPRITE = 6;
const size_t VERTS_PER_SPRITE = 4;
const size_t MAX_SPRITES_PER_BATCH = 32;

struct gl_vertex
{
	GLfloat pos[2];
	GLfloat texcoord[2];
	GLfloat color[4];
};

};

sprite_batch::sprite_batch()
: vbo_verts_ { new gl_buffer_object { GL_ARRAY_BUFFER } }
, vbo_indices_ { new gl_buffer_object { GL_ELEMENT_ARRAY_BUFFER } }
{
	vbo_verts_->bind();
	vbo_verts_->buffer_data(MAX_SPRITES_PER_BATCH*VERTS_PER_SPRITE*sizeof(gl_vertex), nullptr, GL_DYNAMIC_DRAW);
	vbo_verts_->unbind();

	vbo_indices_->bind();
	vbo_indices_->buffer_data(MAX_SPRITES_PER_BATCH*INDICES_PER_SPRITE*sizeof(GLushort), nullptr, GL_STATIC_DRAW);

	auto index_ptr = reinterpret_cast<GLushort *>(vbo_indices_->map(GL_WRITE_ONLY));

	for (int i = 0; i < MAX_SPRITES_PER_BATCH; i++) {
		*index_ptr++ = i*4;
		*index_ptr++ = i*4 + 1;
		*index_ptr++ = i*4 + 2;

		*index_ptr++ = i*4 + 2;
		*index_ptr++ = i*4 + 3;
		*index_ptr++ = i*4;
	}

	vbo_indices_->unmap();
	vbo_indices_->unbind();
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
	sprites_.push_back(
		{ tex,
		  tex_coords,
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

	std::sort(
		std::begin(sorted_sprites),
		std::end(sorted_sprites),
		[](const sprite_info *a, const sprite_info *b) { return a->tex < b->tex; });

	std::stable_sort(
		std::begin(sorted_sprites),
		std::end(sorted_sprites),
		[](const sprite_info *a, const sprite_info *b) { return a->depth < b->depth; });

	enable_alpha_blend _;
	enable_texture __;
	glColor4f(1, 1, 1, 1); // XXX for now

	const texture *batch_tex = nullptr;
	size_t batch_start = 0;

	size_t num_sprites = sorted_sprites.size();

	for (size_t i = 0; i < num_sprites; i++) {
		auto sp = sorted_sprites[i];

		if (sp->tex != batch_tex || i - batch_start == MAX_SPRITES_PER_BATCH) {
			if (i > 0) {
				render(batch_tex, &sorted_sprites[batch_start], i - batch_start);
			}

			batch_tex = sp->tex;
			batch_start = i;
		}
	}

	render(batch_tex, &sorted_sprites[batch_start], num_sprites - batch_start);
}

void
sprite_batch::render(const texture *tex, const sprite_info **sprites, size_t num_sprites)
{
	tex->bind();

	vbo_verts_->bind();

	auto vert_ptr = reinterpret_cast<gl_vertex *>(vbo_verts_->map(GL_WRITE_ONLY));

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

		const auto& tex_coords = sp->tex_coords;

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

	vbo_verts_->unmap();
	vbo_indices_->bind();

	glVertexPointer(2, GL_FLOAT, sizeof(gl_vertex), reinterpret_cast<GLvoid *>(offsetof(gl_vertex, pos)));
	glTexCoordPointer(2, GL_FLOAT, sizeof(gl_vertex), reinterpret_cast<GLvoid *>(offsetof(gl_vertex, texcoord)));
	glColorPointer(4, GL_FLOAT, sizeof(gl_vertex), reinterpret_cast<GLvoid *>(offsetof(gl_vertex, color)));

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glDrawElements(GL_TRIANGLES, num_sprites*INDICES_PER_SPRITE, GL_UNSIGNED_SHORT, 0);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	vbo_indices_->unbind();
	vbo_verts_->unbind();
}

}
