#pragma once

#include "shiny_sprite.h"
#include "entity.h"

namespace ggl {
class sprite;
class sprite_batch;
}

class powerup : public entity
{
public:
	powerup(game& g, const vec2f& pos, const vec2f& dir);

	void draw(ggl::sprite_batch& sb) const override;
	bool update() override;

	bool intersects(const vec2i&, const vec2i&) const override
	{ return false; }

	bool intersects(const vec2i&, float) const override
	{ return false; }

private:
	bool update_moving();
	bool update_sliding();

	enum state { MOVING, SLIDING } state_;
	vec2f pos_;
	vec2f dir_;

	const ggl::sprite *outer_sprite_;
	shiny_sprite text_;
};
