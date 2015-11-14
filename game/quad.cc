#include <algorithm>

#include <ggl/texture.h>
#include <ggl/sprite.h>
#include <ggl/font.h>
#include <ggl/resources.h>
#include <ggl/util.h>

#include "game.h"
#include "quad.h"

quad::quad()
{ }

void
quad::draw(horiz_align ha, vert_align va) const
{
	glPushMatrix();

	float dx, dy;

	switch (ha) {
		case horiz_align::LEFT:
			dx = .5f*get_width();
			break;

		case horiz_align::CENTER:
			dx = 0;
			break;

		case horiz_align::RIGHT:
			dx = -.5f*get_width();
			break;
	}

	switch (va) {
		case vert_align::TOP:
			dy = .5f*get_height();
			break;

		case vert_align::CENTER:
			dy = 0;
			break;

		case vert_align::BOTTOM:
			dy = -.5f*get_height();
			break;
	}

	glTranslatef(dx, dy, 0.f);

	draw_quad();

	glPopMatrix();
}

image_quad::image_quad(const ggl::texture *tex)
: tex_ { tex }
, width_ { tex_->orig_width }
, height_ { tex_->orig_height }
{
	const float x0 = -.5f*width_;
	const float x1 = x0 + width_;

	const float y0 = -.5f*height_;
	const float y1 = y0 + height_;

	const float du = static_cast<float>(width_)/tex_->width;
	const float dv = static_cast<float>(height_)/tex_->height;

	va_.push_back({ x0, y0, 0.f, 0.f });
	va_.push_back({ x0, y1, 0.f, dv });
	va_.push_back({ x1, y0, du, 0.f });
	va_.push_back({ x1, y1, du, dv });
}

unsigned
image_quad::get_width() const
{
	return width_;
}

unsigned
image_quad::get_height() const
{
	return height_;
}

void
image_quad::draw_quad() const
{
	ggl::enable_texture _;

	tex_->bind();
	va_.draw(GL_TRIANGLE_STRIP);
}

text_quad::text_quad(const ggl::font *font, const std::basic_string<wchar_t> text)
: tex_ { nullptr /* font->tex */ }
{
	font->render(text, va_);

	using vert = decltype(va_)::value_type;

	auto cmp_x = [](const vert& a, const vert& b) { return a.pos[0] < b.pos[0]; };
	auto cmp_y = [](const vert& a, const vert& b) { return a.pos[1] < b.pos[1]; };

	auto x_min = std::min_element(std::begin(va_), std::end(va_), cmp_x)->pos[0];
	auto x_max = std::max_element(std::begin(va_), std::end(va_), cmp_x)->pos[0];

	auto y_min = std::min_element(std::begin(va_), std::end(va_), cmp_y)->pos[1];
	auto y_max = std::max_element(std::begin(va_), std::end(va_), cmp_y)->pos[1];

	width_ = x_max - x_min;
	height_ = y_max - y_min;

	const int dx = -x_min - .5*width_;
	const int dy = -y_min - .5*height_;

	for (auto& v : va_) {
		v.pos[0] += dx;
		v.pos[1] += dy;
	}
}

unsigned
text_quad::get_width() const
{
	return width_;
}

unsigned
text_quad::get_height() const
{
	return height_;
}

void
text_quad::draw_quad() const
{
#if 0
	ggl::enable_alpha_blend _;
	ggl::enable_texture __;

	tex_->bind();
	va_.draw(GL_TRIANGLES);
#endif
}

// XXX: we only need game& for game_.tics, should be global somewhere?

shiny_sprite_quad::shiny_sprite_quad(const ggl::sprite *sprite, const game& g, float tex_offset, float speed)
: sprite_ { sprite }
, shine_texture_ { ggl::res::get_texture("images/lives-left-shine.png") }
, game_ { g }
, tex_offset_ { tex_offset }
, speed_ { speed }
{ }

unsigned
shiny_sprite_quad::get_width() const
{
	return sprite_->width;
}

unsigned
shiny_sprite_quad::get_height() const
{
	return sprite_->height;
}

void
shiny_sprite_quad::draw_quad() const
{
	ggl::enable_alpha_blend _;

	const short w = sprite_->width;
	const short h = sprite_->height;

	const float x0 = -.5f*w;
	const float x1 = x0 + w;

	const float y0 = -.5f*h;
	const float y1 = y0 + h;

	const float u0 = sprite_->u0;
	const float u1 = sprite_->u1;

	const float v0 = sprite_->v0;
	const float v1 = sprite_->v1;

	const float s0 = speed_*game_.tics;
	const float s1 = s0 - tex_offset_*static_cast<float>(sprite_->height)/sprite_->width;

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	sprite_->tex->bind();
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	shine_texture_->bind();
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

	// add RGB
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

	// use alpha of first texture
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);

	using va_type = ggl::vertex_array_multitexcoord<GLfloat, 2, GLfloat, 2, GLfloat, 2>;

	(va_type {
		{ x0, y0, u0, v1, s0, 0 },
		{ x1, y0, u1, v1, s0 + tex_offset_, 0 },
		{ x0, y1, u0, v0, s1, 1 },
		{ x1, y1, u1, v0, s1 + tex_offset_, 1 } }).draw(GL_TRIANGLE_STRIP);

	glDisable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
}
