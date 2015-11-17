#include <cassert>
#include <sstream>

#include <ggl/gl.h>
#include <ggl/sprite.h>
#include <ggl/texture.h>
#include <ggl/resources.h>
#include <ggl/font.h>
#include <ggl/util.h>

#include "game.h"
#include "lives_widget.h"

namespace {

const int INTRO_TICS = 30;
const int IDLE_TICS = 90;
const int OUTRO_TICS = 30;

};

lives_widget::lives_widget(game& g)
: widget { g }
, font_ { ggl::res::get_font("fonts/hud-small.spr") }
, hide_ { true }
, circle_ { ggl::res::get_sprite("lives-left-circle.png"), game_, .5, -.02 }
{
	auto& p = game_.get_player();

	respawn_conn_ =
		p.get_respawn_event().connect(
			std::bind(&lives_widget::on_player_respawn, this, std::placeholders::_1));

	death_conn_ =
		p.get_death_event().connect(
			std::bind(&lives_widget::on_player_death, this));

	set_state(state::HIDDEN);
}

void
lives_widget::initialize_text(int lives_left)
{
	std::basic_stringstream<wchar_t> ss;

	if (lives_left > 0)
		ss << lives_left << " LEFT";
	else
		ss << "LAST PLAYER";

	text_ = ss.str();
}

bool
lives_widget::update()
{
	switch (state_) {
		case state::HIDDEN:
			if (!hide_)
				set_state(state::INTRO);
			break;

		case state::INTRO:
			if (++state_tics_ == INTRO_TICS)
				set_state(state::IDLE);
			break;

		case state::IDLE:
			if (++state_tics_ == IDLE_TICS)
				hide_ = true;
			if (hide_)
				set_state(state::OUTRO);
			break;

		case state::OUTRO:
			if (++state_tics_ == OUTRO_TICS)
				set_state(state::HIDDEN);
			break;
	}

	return true;
}

void
lives_widget::draw(ggl::sprite_batch& sb) const
{
	if (state_ == state::HIDDEN)
		return;

	auto pos = game_.get_player_screen_position();

	vec2f circle_scale, text_pos;
	ggl::horiz_align text_ha;
	ggl::vert_align text_va;

	if (pos.x < .5f*game_.viewport_width) {
		circle_scale.x = -1.f;
		text_ha = ggl::horiz_align::RIGHT;
	} else {
		circle_scale.x = 1.f;
		text_ha = ggl::horiz_align::LEFT;
	}

	if (pos.y < .5f*game_.viewport_height) {
		circle_scale.y = -1;
		text_va = ggl::vert_align::TOP;
	} else {
		circle_scale.y = 1;
		text_va = ggl::vert_align::BOTTOM;
	}

	float scale;

	switch (state_) {
		case state::INTRO:
			scale = static_cast<float>(state_tics_)/INTRO_TICS;
			break;

		case state::OUTRO:
			scale = 1.f - static_cast<float>(state_tics_)/OUTRO_TICS;
			break;

		case state::IDLE:
			scale = 1;
			break;
	}

	text_pos = circle_scale*vec2f { -110, -80 };

	sb.push_matrix();

	sb.translate(pos.x, pos.y);
	sb.scale(scale);

	// circle

	sb.push_matrix();
	sb.scale(circle_scale.x, circle_scale.y);
	circle_.draw(sb, 2);
	sb.pop_matrix();

	// text

	font_->draw(sb, 0, text_, text_pos, text_va, text_ha);

	sb.pop_matrix();
}

void
lives_widget::on_player_respawn(int lives_left)
{
	initialize_text(lives_left);
	hide_ = false;
}

void
lives_widget::on_player_death()
{
	hide_ = true;
}

void
lives_widget::set_state(state next_state)
{
	state_ = next_state;
	state_tics_ = 0;
}
