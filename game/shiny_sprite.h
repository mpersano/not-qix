#pragma once

namespace ggl {
class sprite;
class texture;
class sprite_batch;
}

class game;

class shiny_sprite
{
public:
	shiny_sprite(const ggl::sprite *sprite, const game& g, float tex_offset, float speed);

	void draw(ggl::sprite_batch& sb, float depth) const;

private:
	void draw_quad() const;

	const ggl::sprite *sprite_;
	const ggl::texture *shine_texture_;
	const game& game_;
	float tex_offset_;
	float speed_;
};
