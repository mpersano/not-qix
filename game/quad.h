#pragma once

#include <string>
#include <utility>
#include <wchar.h>

#include <ggl/vertex_array.h>
#include <ggl/vec2.h>

namespace ggl {
class texture;
class font;
}

class quad
{
public:
	quad();
	virtual ~quad() = default;

	virtual unsigned get_width() const = 0;
	virtual unsigned get_height() const = 0;

	void draw() const;

	vec2f pos;
	vec2f scale;
	float alpha;

protected:
	virtual void draw_quad() const = 0;
};

class image_quad : public quad
{
public:
	image_quad(const ggl::texture *tex);

	unsigned get_width() const override;
	unsigned get_height() const override;

private:
	void draw_quad() const override;

	const ggl::texture *tex_;
	ggl::vertex_array_texcoord<GLfloat, 2, GLfloat, 2> va_;
	unsigned width_, height_;
};

class text_quad : public quad
{
public:
	text_quad(const ggl::font *font, const std::basic_string<wchar_t> text);

	unsigned get_width() const override;
	unsigned get_height() const override;

private:
	void draw_quad() const override;

	const ggl::texture *tex_;
	ggl::vertex_array_texcoord<GLshort, 2, GLfloat, 2> va_;
	std::pair<vec2s, vec2s> rect_;
};
