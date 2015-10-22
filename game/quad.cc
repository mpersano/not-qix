#include <algorithm>

#include <ggl/texture.h>
#include <ggl/font.h>
#include <ggl/util.h>

#include "quad.h"

quad::quad()
: pos { 0.f, 0.f }
, scale { 1.f, 1.f }
, alpha { 1.f }
{ }

void
quad::draw(horiz_align ha, vert_align va) const
{
	glPushMatrix();
	glTranslatef(pos.x, pos.y, 0.f);
	glScalef(scale.x, scale.y, 1.f);

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

	glColor4f(1, 1, 1, alpha);

	draw_quad();

	glPopMatrix();
}

void
quad::draw() const
{
	draw(horiz_align::CENTER, vert_align::CENTER);
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
: tex_ { font->tex }
{
	font->render(text, va_);

	using vert = decltype(va_)::value_type;

	auto cmp_x = [](const vert& a, const vert& b) { return a.pos[0] < b.pos[0]; };
	auto cmp_y = [](const vert& a, const vert& b) { return a.pos[1] < b.pos[1]; };

	auto x_min = std::min_element(std::begin(va_), std::end(va_), cmp_x)->pos[0];
	auto x_max = std::max_element(std::begin(va_), std::end(va_), cmp_x)->pos[0];

	auto y_min = std::min_element(std::begin(va_), std::end(va_), cmp_y)->pos[1];
	auto y_max = std::max_element(std::begin(va_), std::end(va_), cmp_y)->pos[1];

	rect_ = std::make_pair(vec2s { x_min, y_min }, vec2s { x_max, y_max });
}

unsigned
text_quad::get_width() const
{
	return rect_.second.x - rect_.first.x;
}

unsigned
text_quad::get_height() const
{
	return rect_.second.y - rect_.first.y;
}

void
text_quad::draw_quad() const
{
	ggl::enable_alpha_blend _;
	ggl::enable_texture __;

	glPushMatrix();

	glTranslatef(
		-rect_.first.x - .5f*get_width(),
		-rect_.first.y - .5f*get_height(),
		0);

	tex_->bind();
	va_.draw(GL_TRIANGLES);

	glPopMatrix();
}
