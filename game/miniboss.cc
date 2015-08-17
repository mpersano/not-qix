#include <ggl/sprite.h>
#include <ggl/resources.h>

#include "game.h"
#include "miniboss.h"

namespace {

const float SPEED = 1;
const float RADIUS = 16;

} // (anonymous namespace)

miniboss::miniboss(game& g, const vec2f& pos, const vec2f& dir, const std::function<void(void)>& on_death)
: phys_foe { g, pos, dir, SPEED, RADIUS }
, sprite_ { ggl::res::get_sprite("miniboss.png") }
, on_death_ { on_death }
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
				on_death_();
				return false;
			}
		}
	}

	move();

	return true;
}
