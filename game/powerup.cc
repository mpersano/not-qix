#include <cmath>
#include <cassert>
#include <algorithm>

#include <ggl/gl.h>
#include <ggl/sprite.h>
#include <ggl/action.h>
#include <ggl/resources.h>

#include "game.h"
#include "effect.h"
#include "quad.h"
#include "powerup.h"

namespace {

const int RADIUS = 12;
const float SPEED = 1.5;

class picked_effect : public effect
{
public:
	picked_effect(const vec2f& pos);

	bool update() override;
	void draw() const override;

	bool is_position_absolute() const override
	{ return true; }

private:
	vec2f pos_;
	text_quad text_;

	float delta_y_;
	float text_alpha_;

	ggl::action_ptr action_;
};

picked_effect::picked_effect(const vec2f& pos)
: pos_ { pos }
, text_ { ggl::res::get_font("fonts/tiny.spr"), L"power up!" }
, action_ { ggl::res::get_action("animations/powerup.xml") }
{
	action_->bind("delta-y", &delta_y_);
	action_->bind("text-alpha", &text_alpha_);
	action_->set_properties();
}

bool
picked_effect::update()
{
	action_->update();
	return !action_->done();
}

void
picked_effect::draw() const
{
	glColor4f(1, 1, 1, text_alpha_);

	glPushMatrix();
	glTranslatef(pos_.x, pos_.y + delta_y_, 0.f);
	text_.draw();
	glPopMatrix();
}

};

powerup::powerup(game& g, const vec2f& pos, const vec2f& dir)
: entity { g }
, pos_ { pos }
, dir_ { dir }
, state_ { state::MOVING }
, outer_sprite_ { ggl::res::get_sprite("powerup-outer.png") }
, text_ { new shiny_sprite_quad { ggl::res::get_sprite("powerup-inner.png"), game_, .5, -.02 } }
{ }

void
powerup::draw() const
{
	glPushMatrix();
	glTranslatef(pos_.x, pos_.y, 0);

	glPushMatrix();
	glRotatef(3.f*game_.tics, 0, 0, 1);
	outer_sprite_->draw(ggl::sprite::horiz_align::CENTER, ggl::sprite::vert_align::CENTER);
	glPopMatrix();

	text_->draw();

	glPopMatrix();
}

bool
powerup::update()
{
	bool done = false;

	switch (state_) {
		case state::MOVING:
			done = update_moving();
			break;

		case state::SLIDING:
			done = update_sliding();
			break;
	}

	if (done) {
		game_.add_effect(
			std::unique_ptr<effect>(new picked_effect(pos_ + vec2f(game_.offset))));
		printf("powerup collected!\n");
	}

	return !done;
}

bool
powerup::update_moving()
{
	const int r0 = static_cast<int>(pos_.y/CELL_SIZE);
	const int c0 = static_cast<int>(pos_.x/CELL_SIZE);

	// covered?

	if (game_(c0, r0))
		return true;

	auto next_pos = pos_ + SPEED*dir_;

	// collision with filled area

	const int r1 = static_cast<int>(next_pos.y/CELL_SIZE);
	const int c1 = static_cast<int>(next_pos.x/CELL_SIZE);

	if (c0 != c1) {
		int cm = c0 < c1 ? c1 : c0;
		float xm = cm*CELL_SIZE;
		assert((xm >= pos_.x && xm <= next_pos.x) || (xm >= next_pos.x && xm <= pos_.x));
		float ym = (next_pos.y - pos_.y)*(xm - pos_.x)/(next_pos.x - pos_.x) + pos_.y;
		const int r = static_cast<int>(ym/CELL_SIZE);

		if (game_(c0, r) != game_(c1, r)) {
			pos_ = vec2f { xm, ym };
			state_ = state::SLIDING;
			dir_ = vec2f { 0, 1 };
			return false;
		}
	}

	if (r0 != r1) {
		int rm = r0 < r1 ? r1 : r0;
		float ym = rm*CELL_SIZE;
		assert((ym >= pos_.y && ym <= next_pos.y) || (ym >= next_pos.y && ym <= pos_.y));
		float xm = (next_pos.x - pos_.x)*(ym - pos_.y)/(next_pos.y - pos_.y) + pos_.x;
		const int c = static_cast<int>(xm/CELL_SIZE);

		if (game_(c, r0) != game_(c, r1)) {
			pos_ = vec2f { xm, ym };
			state_ = state::SLIDING;
			dir_ = vec2f { 1, 0 };
			return false;
		}
	}

	pos_ = next_pos;

	// collide with viewport limits

	const vec2i v0 = -game_.offset;
	const vec2i v1 {
		std::min(game_.grid_cols*CELL_SIZE, v0.x + game_.viewport_width),
			std::min(game_.grid_rows*CELL_SIZE, v0.y + game_.viewport_height) };

	if (dir_.x < 0 && pos_.x < v0.x + RADIUS)
		dir_.x = -dir_.x;

	if (dir_.x > 0 && pos_.x > v1.x - RADIUS)
		dir_.x = -dir_.x;

	if (dir_.y < 0 && pos_.y < v0.y + RADIUS)
		dir_.y = -dir_.y;

	if (dir_.y > 0 && pos_.y > v1.y - RADIUS)
		dir_.y = -dir_.y;

	return false;
}

bool
powerup::update_sliding()
{
	// caught by the player?

	if (distance(pos_, vec2f(game_.get_player_world_position())) < 4.f)
		return true;

	auto next_pos = pos_ + SPEED*dir_;

	const int r0 = static_cast<int>(pos_.y/CELL_SIZE);
	const int c0 = static_cast<int>(pos_.x/CELL_SIZE);

	const int r1 = static_cast<int>(next_pos.y/CELL_SIZE);
	const int c1 = static_cast<int>(next_pos.x/CELL_SIZE);

	// collision with filled area

	if (c0 != c1) {
		int c = c0 < c1 ? c1 : c0;
		int r = r0;

		if (game_(c1, r) == game_(c1, r - 1)) {
			// change direction

			next_pos = vec2f { c, r }*CELL_SIZE;

			if (game_(c - 1, r) != game_(c, r)) {
				dir_ = vec2f { 0, 1 };
			} else if (game_(c - 1, r - 1) != game_(c, r - 1)) {
				dir_ = vec2f { 0, -1 };
			} else {
				return true;
			}
		}
	} else if (r0 != r1) {
		int r = r0 < r1 ? r1 : r0;
		int c = c0;

		if (game_(c - 1, r1) == game_(c, r1)) {
			// change direction

			next_pos = vec2f { c, r }*CELL_SIZE;

			if (game_(c - 1, r) != game_(c - 1, r - 1)) {
				dir_ = vec2f { -1, 0 };
			} else if (game_(c, r) != game_(c, r - 1)) {
				dir_ = vec2f { 1, 0 };
			} else {
				return true;
			}
		}
	} else {
		// covered?

		if (dir_.x != 0.f) {
			assert(dir_.y == 0.f);

			if (game_(c0, r0) == game_(c0, r0 - 1))
				return true;
		} else if (dir_.y != 0.f) {
			assert(dir_.x == 0.f);

			if (game_(c0 - 1, r0) == game_(c0, r0))
				return true;
		}
	}

	pos_ = next_pos;

	return false;
}
