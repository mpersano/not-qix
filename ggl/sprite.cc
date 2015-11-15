#include <ggl/texture.h>
#include <ggl/vertex_array.h>
#include <ggl/util.h>
#include <ggl/sprite.h>
#include <ggl/sprite_batch.h>

namespace ggl {

sprite::sprite(const texture *tex, int u, int v, int width, int height)
: tex { tex }
, width { width }
, height { height }
{
	const int tex_width = tex->width;
	const int tex_height = tex->height;

	const float du = static_cast<float>(width)/tex_width;
	const float dv = static_cast<float>(height)/tex_height;

	u0 = static_cast<float>(u)/tex_width;
	u1 = u0 + du;

	v0 = static_cast<float>(tex_height - v)/tex_height;
	v1 = v0 - dv;
}

void
sprite::draw() const
{
	draw(0, 0);
}

void
sprite::draw(float x, float y) const
{
	draw(x, y, x + width, y + height);
}

void
sprite::draw(horiz_align ha, vert_align va) const
{
	draw(0, 0, ha, va);
}

void
sprite::draw(float x, float y, horiz_align ha, vert_align va) const
{
	float x0, y0;

	switch (ha) {
		case horiz_align::LEFT:
			x0 = x;
			break;

		case horiz_align::CENTER:
			x0 = x - .5f*width;
			break;

		case horiz_align::RIGHT:
			x0 = x - width;
			break;
	}

	switch (va) {
		case vert_align::BOTTOM:
			y0 = y;
			break;

		case vert_align::CENTER:
			y0 = y - .5f*height;
			break;

		case vert_align::TOP:
			y0 = y - height;
			break;
	}

	draw(x0, y0, x0 + width, y0 + height);
}

void
sprite::draw(float x0, float y0, float x1, float y1) const
{
	enable_alpha_blend _;
	enable_texture __;

	glColor4f(1, 1, 1, 1);

	tex->bind();

	(vertex_array_texcoord<GLfloat, 2, GLfloat, 2>
		{ { x0, y0, u0, v1 },
		  { x1, y0, u1, v1 },
		  { x0, y1, u0, v0 },
		  { x1, y1, u1, v0 } }).draw(GL_TRIANGLE_STRIP);
}

void
sprite::draw(sprite_batch& sb, float depth) const
{
	draw(sb, depth, { 0.f, 0.f });
}

void
sprite::draw(sprite_batch& sb, float depth, const vec2f& pos) const
{
	draw(sb, depth, pos, vert_align::CENTER, horiz_align::CENTER);
}

void
sprite::draw(sprite_batch& sb, float depth, const vec2f& pos, vert_align va, horiz_align ha) const
{
	vec2f p0 = pos;

	switch (ha) {
		case horiz_align::LEFT:
			break;

		case horiz_align::CENTER:
			p0.x -= .5f*width;
			break;

		case horiz_align::RIGHT:
			p0.x -= width;
			break;
	}

	switch (va) {
		case vert_align::BOTTOM:
			break;

		case vert_align::CENTER:
			p0.y -= .5f*height;
			break;

		case vert_align::TOP:
			p0.y -= height;
			break;
	}

	vec2f p1 = p0 + vec2f { width, height };

	sb.draw(tex, { { u0, v1 }, { u1, v0 } }, bbox { p0, p1 }, depth );
}

}
