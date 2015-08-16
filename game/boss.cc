#include <cmath>

#include "tween.h"
#include "game.h"
#include "boss.h"

namespace {

static const float BOSS_SPEED = 2;

class bullet : public foe
{
public:
	bullet(game& g, const vec2f& pos, const vec2f& dir);

	void draw() const;
	bool update();

	bool is_boss() const
	{ return false; }

	bool intersects(const vec2i& from, const vec2i& to) const override;

private:
	static const int LENGTH = 20;

	vec2f pos_;
	vec2f dir_;;
};

bullet::bullet(game& g, const vec2f& pos, const vec2f& dir)
: foe { g }
, pos_ { pos }
, dir_ { dir }
{ }

void
bullet::draw() const
{
	glColor4f(1, 0, 0, 1);

	glBegin(GL_LINES);
	glVertex2f(pos_.x, pos_.y);
	glVertex2f(pos_.x + LENGTH*dir_.x, pos_.y + LENGTH*dir_.y);
	glEnd();
}

bool
bullet::update()
{
	static const float SPEED = 4;

	pos_ += SPEED*dir_;

	// outside grid?

	int grid_width = game_.grid_cols*CELL_SIZE;
	int grid_height = game_.grid_rows*CELL_SIZE;

	vec2f end = pos_ + dir_*static_cast<float>(LENGTH);

	if (std::min(pos_.x, end.x) > grid_width ||
		std::max(pos_.x, end.x) < 0 ||
		std::min(pos_.y, end.y) > grid_height ||
		std::max(pos_.y, end.y) < 0) {
		return false;
	}

	return true;
}

bool
bullet::intersects(const vec2i& from, const vec2i& to) const
{
	vec2f end = pos_ + dir_*static_cast<float>(LENGTH);

	// i'm sure there's a cleaner way to do this...

	float d = (end.y - pos_.y)*(to.x - from.x) - (end.x - pos_.x)*(to.y - from.y);
	if (d == 0)
		return false;

	float ua = ((end.x - pos_.x)*(from.y - pos_.y) - (end.y - pos_.y)*(from.x - pos_.x))/d;
	float ub = ((to.x - from.x)*(from.y - pos_.y) - (to.y - from.y)*(from.x - pos_.x))/d;

	return ua >= 0 && ua <= 1 && ub >= 0 && ub <= 1;
}

} // (anonymous namespace)

//
//  b o s s
//

boss::boss(game& g)
: phys_foe { g, vec2f { 100, 100 }, normalized(vec2f { 1.5f, .5f }), BOSS_SPEED, 30 }
, spike_angle_ { 0 }
{
	set_state(state::CHASING);
}

void
boss::set_state(state next_state)
{
	state_ = next_state;
	state_tics_ = 0;
}

bool
boss::update()
{
	move();

	++state_tics_;

	switch (state_) {
		case state::CHASING:
			update_chasing();
			break;

		case state::PRE_FIRING:
			update_pre_firing();
			break;

		case state::FIRING:
			update_firing();
			break;

		case state::POST_FIRING:
			update_post_firing();
			break;
	}

	return true;
}

void
boss::update_chasing()
{
	chase_player();

	spike_angle_ += .025f;

	if (state_tics_ >= MIN_CHASE_TICS) {
		if (rand()%16 == 0)
			set_state(state::PRE_FIRING);
	}
}

void
boss::update_pre_firing()
{
	chase_player();

	aim_player();

	speed = exp_tween<float>()(BOSS_SPEED, 0, static_cast<float>(state_tics_)/PRE_FIRING_TICS);

	if (state_tics_ >= PRE_FIRING_TICS)
		set_state(state::FIRING);
}

void
boss::update_firing()
{
	if (state_tics_%30 == 0) {
		const float a = spike_angle_;
		const vec2f tip = pos + vec2f { cosf(a), sinf(a) }*radius*1.2f;
		vec2f dir = normalized(tip - pos);
		game_.add_foe(std::unique_ptr<foe>(new bullet { game_, tip, dir }));
	}

	aim_player();

	if (state_tics_ >= FIRING_TICS)
		set_state(state::POST_FIRING);
}

void
boss::update_post_firing()
{
	chase_player();

	speed = exp_tween<float>()(BOSS_SPEED, 0, 1.f - static_cast<float>(state_tics_)/POST_FIRING_TICS);

	if (state_tics_ >= POST_FIRING_TICS)
		set_state(state::CHASING);
}

void
boss::aim_player()
{
	const vec2f d = vec2f(game_.get_player_world_position()) - pos;
	const vec2f n = normalized(vec2f { -d.y, d.x });

	// rotate spike towards player

	const float a = spike_angle_;
	vec2f u { cosf(a), sinf(a) };
	spike_angle_ -= .05f*dot(n, u);
}

void
boss::chase_player()
{
	vec2f n { -dir.y, dir.x };

	float da = .025f*dot(n, normalized(vec2f(game_.get_player_world_position()) - pos));

	const float c = cosf(da);
	const float s = sinf(da);

	vec2f next_dir { dot(dir, vec2f { c, -s }), dot(dir, vec2f { s, c }) };
	dir = next_dir;
}

void
boss::draw() const
{
	glDisable(GL_TEXTURE_2D);
	glColor4f(1, 1, 0, 1);

	draw_core();
	draw_spikes();
}

void
boss::draw_core() const
{
	static const int NUM_SEGS = 13;

	float a = 0;
	const float da = 2.f*M_PI/NUM_SEGS;

	glBegin(GL_LINE_LOOP);

	for (int i = 0; i < NUM_SEGS; i++) {
		vec2f p = pos + vec2f { cosf(a), sinf(a) }*radius;
		glVertex2f(p.x, p.y);
		a += da;
	}

	glEnd();
}

void
boss::draw_spikes() const
{
	switch (state_) {
		case state::CHASING:
			{
			const float da = 2.f*M_PI/NUM_SPIKES;
			float a = spike_angle_;

			for (int i = 0; i < NUM_SPIKES; i++) {
				draw_spike(a);
				a += da;
			}
			}
			break;

		case state::PRE_FIRING:
		case state::POST_FIRING:
			{
			float t;

			if (state_ == state::PRE_FIRING)
				t = static_cast<float>(state_tics_)/PRE_FIRING_TICS;
			else
				t = 1.f - static_cast<float>(state_tics_)/POST_FIRING_TICS;

			const float da = 2.f*M_PI/NUM_SPIKES;

			for (size_t i = 0; i <= NUM_SPIKES/2; i++)
				draw_spike(spike_angle_ + exp_tween<float>()(i*da, 0, t));

			for (size_t i = 0; i < NUM_SPIKES/2; i++)
				draw_spike(spike_angle_ - exp_tween<float>()((i + 1)*da, 0, t));
			}
			break;

		case state::FIRING:
			draw_spike(spike_angle_);
			break;
	}
}

void
boss::draw_spike(float a) const
{
	glPushMatrix();

	glTranslatef(pos.x, pos.y, 0.f);
	glRotatef(a*180.f/M_PI - 90.f, 0, 0, 1);
	glTranslatef(0, radius*1.2f, 0.f);

	glBegin(GL_LINE_LOOP);
	glVertex2i(-5, 0);
	glVertex2i(0, 15);
	glVertex2i(5, 0);
	glEnd();

	glPopMatrix();
}
