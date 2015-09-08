#include <ggl/texture.h>
#include <ggl/font.h>

#include "quad.h"

quad::quad()
: pos { 0.f, 0.f }
, scale { 1.f, 1.f }
{ }

void
quad::draw() const
{
	glPushMatrix();
	glTranslatef(pos.x, pos.y, 0.f);
	glScalef(scale.x, scale.y, 1.f);

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
	tex_->bind();
	va_.draw(GL_TRIANGLE_STRIP);
}

text_quad::text_quad(const ggl::font *font, const std::basic_string<wchar_t> text)
: tex_ { font->tex }
{
	font->render(text, va_);

	vec2s v0 { va_.front().pos[0], va_.front().pos[1] };
	vec2s v1 = v0;

	for (auto& v : va_) {
		v0.x = std::min(v0.x, v.pos[0]);
		v0.y = std::min(v0.y, v.pos[1]);

		v1.x = std::max(v1.x, v.pos[0]);
		v1.y = std::max(v1.y, v.pos[1]);
	}

	rect_ = std::make_pair(v0, v1);
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
	glTranslatef(
		-rect_.first.x - .5f*get_width(),
		-rect_.first.y - .5f*get_height(),
		0);

	tex_->bind();
	va_.draw(GL_TRIANGLES);
}
