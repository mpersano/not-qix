#include <ggl/texture.h>
#include <ggl/util.h>
#include <ggl/sprite.h>
#include <ggl/render.h>

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
sprite::draw(float depth) const
{
	draw(depth, { 0.f, 0.f });
}

void
sprite::draw(float depth, const vec2f& pos) const
{
	draw(depth, pos, vert_align::CENTER, horiz_align::CENTER);
}

void
sprite::draw(float depth, const vec2f& pos, vert_align va, horiz_align ha) const
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

	render::draw(tex, { { u0, v1 }, { u1, v0 } }, bbox { p0, p1 }, depth );
}

}
