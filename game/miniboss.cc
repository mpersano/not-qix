#include <ggl/sprite.h>
#include <ggl/resources.h>

#include "game.h"
#include "boss.h"
#include "miniboss.h"

namespace {

const float SPEED = 1;
const float RADIUS = 16;

} // (anonymous namespace)

miniboss::miniboss(game& g, boss *b, const vec2f& pos, const vec2f& dir)
: foe { g, pos, dir, SPEED, RADIUS }
, boss_ { b }
, sprite_ { ggl::res::get_sprite("miniboss.png") }
{ }

void
miniboss::draw() const
{
	sprite_->draw(pos_.x, pos_.y, ggl::sprite::horiz_align::CENTER, ggl::sprite::vert_align::CENTER);
}

bool
miniboss::update()
{
	// killed?

	auto p0 = vec2i((pos_ - vec2f { .5f, .5f }*radius_)/CELL_SIZE);
	auto p1 = vec2i((pos_ + vec2f { .5f, .5f }*radius_)/CELL_SIZE);

	for (int r = p0.y; r <= p1.y; r++) {
		for (int c = p0.x; c <= p1.x; c++) {
			if (game_.grid[r*game_.grid_cols + c]) {
				printf("killed!\n");
				boss_->on_miniboss_killed();
				return false;
			}
		}
	}

	update_position();

	return true;
}
