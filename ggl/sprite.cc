#include <ggl/texture.h>
#include <ggl/sprite.h>
#include <ggl/vertex_array.h>

namespace ggl {

sprite::sprite(const texture *tex, int left, int top, int width, int height)
: tex { tex }
, width { width }
, height { height }
{
	const int tex_width = tex->width;
	const int tex_height = tex->height;

	const float du = static_cast<float>(width)/tex_width;
	const float dv = static_cast<float>(height)/tex_height;

	u0 = static_cast<float>(left)/tex_width;
	u1 = u0 + du;

	v0 = static_cast<float>(tex_height - top)/tex_height;
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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);

	glColor4f(1, 1, 1, 1);

	tex->bind();

	(vertex_array_texcoord<GLfloat, 2, GLfloat, 2>
		{ { x0, y0, u0, v1 },
		  { x1, y0, u1, v1 },
		  { x0, y1, u0, v0 },
		  { x1, y1, u1, v0 } }).draw(GL_TRIANGLE_STRIP);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

}
