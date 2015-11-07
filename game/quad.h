#pragma once

#include <string>
#include <utility>
#include <memory>

#include <wchar.h>

#include <ggl/vertex_array.h>
#include <ggl/vec2.h>

namespace ggl {
class sprite;
class texture;
class font;
}

class game;

class quad
{
public:
	quad();
	virtual ~quad() = default;

	virtual unsigned get_width() const = 0;
	virtual unsigned get_height() const = 0;

	enum class vert_align { TOP, CENTER, BOTTOM };
	enum class horiz_align { LEFT, CENTER, RIGHT };

	void draw(horiz_align ha = horiz_align::CENTER, vert_align va = vert_align::CENTER) const;

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
	int width_, height_;
};

class shiny_sprite_quad : public quad
{
public:
	shiny_sprite_quad(const ggl::sprite *sprite, const game& g, float tex_offset, float speed);

	unsigned get_width() const override;
	unsigned get_height() const override;

private:
	void draw_quad() const override;

	const ggl::sprite *sprite_;
	const ggl::texture *shine_texture_;
	const game& game_;
	float tex_offset_;
	float speed_;
};
