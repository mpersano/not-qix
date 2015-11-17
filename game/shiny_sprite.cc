#include <algorithm>

#include <ggl/texture.h>
#include <ggl/sprite.h>
#include <ggl/resources.h>
#include <ggl/util.h>

#include "game.h"
#include "shiny_sprite.h"

// XXX: we only need game& for game_.tics, should be global somewhere?

shiny_sprite::shiny_sprite(const ggl::sprite *sprite, const game& g, float tex_offset, float speed)
: sprite_ { sprite }
, shine_texture_ { ggl::res::get_texture("images/lives-left-shine.png") } // XXX hardcoded?
, game_ { g }
, tex_offset_ { tex_offset }
, speed_ { speed }
{ }

void
shiny_sprite::draw(ggl::sprite_batch& sb, float depth) const
{
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
	const float s1 = s0 + tex_offset_;

	const float t0 = 0;
	const float t1 = tex_offset_*static_cast<float>(sprite_->height)/sprite_->width;

	sb.draw(sprite_->tex,
		shine_texture_,
		{ { u0, v1 }, { u1, v0 } },
		{ { s0, t0 }, { s1, t1 } },
		ggl::bbox { { x0, y0 }, { x1, y1 } },
		depth);
}
