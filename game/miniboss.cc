#include <ggl/sprite.h>
#include <ggl/resources.h>

#include "game.h"
#include "boss.h"
#include "miniboss.h"

miniboss::miniboss(game& g, const vec2f& pos, boss *parent)
: foe { g, pos, RADIUS }
, parent_ { parent }
, sprite_ { ggl::res::get_sprite("miniboss.png") }
, script_thread_ { create_script_thread("scripts/miniboss.lua") }
{
	script_thread_->call("init", this);
}

void
miniboss::draw() const
{
	sprite_->draw(0, pos_);
}

bool
miniboss::update()
{
	// check if killed

	auto p0 = vec2i((pos_ - vec2f { .5f, .5f }*radius_)/CELL_SIZE);
	auto p1 = vec2i((pos_ + vec2f { .5f, .5f }*radius_)/CELL_SIZE);

	for (int r = p0.y; r <= p1.y; r++) {
		for (int c = p0.x; c <= p1.x; c++) {
			if (game_.grid[r*game_.grid_cols + c]) {
				printf("killed!\n");
				parent_->on_miniboss_killed();
				return false;
			}
		}
	}

	script_thread_->call("update", this);

	return true;
}

bool
miniboss::intersects_children(const vec2i& from, const vec2i& to) const
{
	return false;
}

bool
miniboss::intersects_children(const vec2i& center, float radius) const
{
	return false;
}
