#include <algorithm>

#include <ggl/util.h>
#include <ggl/gl_buffer.h>
#include <ggl/texture.h>
#include <ggl/gl_check.h>
#include <ggl/sprite_batch.h>

namespace {

const char *vert_shader_single =
	"uniform mat4 proj_modelview;\n"
	"\n"
	"attribute vec2 position;\n"
	"attribute vec2 texcoord;\n"
	"attribute vec4 color;\n"
	"\n"
	"varying vec2 frag_texcoord;\n"
	"varying vec4 frag_color;\n"
	"\n"
	"void main(void)\n"
	"{\n"
	"	gl_Position = proj_modelview*vec4(position, 0, 1);\n"
	"	frag_texcoord = texcoord;\n"
	"	frag_color = color;\n"
	"}";

const char *frag_shader_single =
	"uniform sampler2D texture;\n"
	"\n"
	"varying vec2 frag_texcoord;\n"
	"varying vec4 frag_color;\n"
	"\n"
	"void main(void)\n"
	"{\n"
	"	gl_FragColor = texture2D(texture, frag_texcoord)*frag_color;\n"
	"}";

const char *vert_shader_multi =
	"uniform mat4 proj_modelview;\n"
	"\n"
	"attribute vec2 position;\n"
	"attribute vec2 texcoord0;\n"
	"attribute vec2 texcoord1;\n"
	"attribute vec4 color;\n"
	"\n"
	"varying vec2 frag_texcoord0;\n"
	"varying vec2 frag_texcoord1;\n"
	"varying vec4 frag_color;\n"
	"\n"
	"void main(void)\n"
	"{\n"
	"	gl_Position = proj_modelview*vec4(position, 0, 1);\n"
	"	frag_texcoord0 = texcoord0;\n"
	"	frag_texcoord1 = texcoord1;\n"
	"	frag_color = color;\n"
	"}";

const char *frag_shader_multi =
	"uniform sampler2D texture0;\n"
	"uniform sampler2D texture1;\n"
	"\n"
	"varying vec2 frag_texcoord0;\n"
	"varying vec2 frag_texcoord1;\n"
	"varying vec4 frag_color;\n"
	"\n"
	"void main(void)\n"
	"{\n"
	"	vec4 c0 = texture2D(texture0, frag_texcoord0);\n"
	"	vec4 c1 = texture2D(texture1, frag_texcoord1);\n"
	"	gl_FragColor = vec4(c0.rgb + c1.rgb, c0.a)*frag_color;\n"
	"}";

}

namespace ggl {

namespace {

const size_t INDICES_PER_SPRITE = 6;
const size_t VERTS_PER_SPRITE = 4;
const size_t MAX_SPRITES_PER_BATCH = 32;

struct gl_vertex_single
{
	GLfloat position[2];
	GLfloat texcoord[2];
	GLfloat color[4];
};

struct gl_vertex_multi
{
	GLfloat position[2];
	GLfloat texcoord0[2];
	GLfloat texcoord1[2];
	GLfloat color[4];
};

};

sprite_batch::sprite_batch()
: vert_buffer_ { GL_ARRAY_BUFFER }
, index_buffer_ { GL_ELEMENT_ARRAY_BUFFER }
, proj_modelview_ { mat4::identity() }
{
	init_buffers();
	init_programs();
	init_vaos();
}

void
sprite_batch::set_viewport(const bbox& viewport)
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
sprite_batch::init_buffers()
{
	// vertex buffer

	vert_buffer_.bind();
	vert_buffer_.buffer_data(MAX_SPRITES_PER_BATCH*VERTS_PER_SPRITE*sizeof(gl_vertex_multi), nullptr, GL_DYNAMIC_DRAW);
	vert_buffer_.unbind();

	// index buffer

	index_buffer_.bind();
	index_buffer_.buffer_data(MAX_SPRITES_PER_BATCH*INDICES_PER_SPRITE*sizeof(GLushort), nullptr, GL_STATIC_DRAW);

	auto index_ptr = reinterpret_cast<GLushort *>(index_buffer_.map(GL_WRITE_ONLY));

	for (int i = 0; i < MAX_SPRITES_PER_BATCH; i++) {
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
sprite_batch::init_programs()
{
	auto init_program = [](gl_program& prog, const char *vert_source, const char *frag_source)
		{
			gl_shader vert_single { GL_VERTEX_SHADER };
			vert_single.set_source(vert_source);
			vert_single.compile();

			gl_shader frag_single { GL_FRAGMENT_SHADER };
			frag_single.set_source(frag_source);
			frag_single.compile();

			prog.attach(vert_single);
			prog.attach(frag_single);
			prog.link();
		};

	init_program(program_single_, vert_shader_single, frag_shader_single);
	init_program(program_multi_, vert_shader_multi, frag_shader_multi);
}

void
sprite_batch::init_vaos()
{
#define INIT_ATTRIB_POINTER(prog, st, field, size) \
	{ \
		GLint location = prog.get_attribute_location(#field); \
		gl_check(glVertexAttribPointer(location, size, GL_FLOAT, GL_FALSE, sizeof(st), reinterpret_cast<const GLvoid *>(offsetof(st, field)))); \
		gl_check(glEnableVertexAttribArray(location)); \
	}

	// vao for single texture sprites

	vao_single_.bind();
	vert_buffer_.bind();
	INIT_ATTRIB_POINTER(program_single_, gl_vertex_single, position, 2)
	INIT_ATTRIB_POINTER(program_single_, gl_vertex_single, texcoord, 2)
	INIT_ATTRIB_POINTER(program_single_, gl_vertex_single, color, 4)

	// vao for multi texture sprites

	vao_multi_.bind();
	vert_buffer_.bind(); // XXX need this?
	INIT_ATTRIB_POINTER(program_multi_, gl_vertex_multi, position, 2)
	INIT_ATTRIB_POINTER(program_multi_, gl_vertex_multi, texcoord0, 2)
	INIT_ATTRIB_POINTER(program_multi_, gl_vertex_multi, texcoord1, 2)
	INIT_ATTRIB_POINTER(program_multi_, gl_vertex_multi, color, 4)

	gl_vertex_array::unbind();

#undef INIT_ATTRIB_POINTER
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

	// sort sprites

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

	// initialize program uniforms

	program_single_.use();
	program_single_.set_uniform_i("texture", 0); // texunit 0
	program_single_.set_uniform_mat4("proj_modelview", proj_modelview_);

	program_multi_.use();
	program_multi_.set_uniform_i("texture0", 0); // texunit 0
	program_multi_.set_uniform_i("texture1", 1); // texunit 0
	program_multi_.set_uniform_mat4("proj_modelview", proj_modelview_);

	// do the dance, do the dance

	enable_alpha_blend _;

	vert_buffer_.bind();

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

	// cleanup

	vert_buffer_.unbind();
	index_buffer_.unbind();

	gl_vertex_array::unbind();

	glUseProgram(0);

	glActiveTexture(GL_TEXTURE0);
}

void
sprite_batch::render(const texture *tex0, const texture *tex1, const sprite_info *const *sprites, size_t num_sprites)
{
	glActiveTexture(GL_TEXTURE0);
	tex0->bind();

	glActiveTexture(GL_TEXTURE1);
	tex1->bind();

	program_multi_.use();

	auto vert_ptr = reinterpret_cast<gl_vertex_multi *>(vert_buffer_.map(GL_WRITE_ONLY));

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
	glDrawElements(GL_TRIANGLES, num_sprites*INDICES_PER_SPRITE, GL_UNSIGNED_SHORT, 0);
}

void
sprite_batch::render(const texture *tex, const sprite_info *const *sprites, size_t num_sprites)
{
	glActiveTexture(GL_TEXTURE0);
	tex->bind();

	program_single_.use();

	auto vert_ptr = reinterpret_cast<gl_vertex_single *>(vert_buffer_.map(GL_WRITE_ONLY));

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
	glDrawElements(GL_TRIANGLES, num_sprites*INDICES_PER_SPRITE, GL_UNSIGNED_SHORT, 0);
}

}
