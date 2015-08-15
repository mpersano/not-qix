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

private:
	static const int LENGTH = 20;

	vec2f pos_;
	vec2f dir_;;
};

class state_chasing : public boss::state
{
public:
	state_chasing(game& g);

	void update(boss& b) override;

private:
	static const int MIN_STATE_TICS = 180;
};

class state_aiming : public boss::state
{
public:
	state_aiming(game& g);

	void update(boss& b) override;

private:
	static const int AIMING_TICS = 90;
};

class state_firing : public boss::state
{
public:
	state_firing(game& g);

	void update(boss& b) override;

private:
	static const int FIRING_TICS = 180;
};

class state_post_firing : public boss::state
{
public:
	state_post_firing(game& g);

	void update(boss& b) override;

private:
	static const int POST_FIRING_TICS = 90;
};

//
//  c h a s i n g
//

state_chasing::state_chasing(game& g)
: boss::state { g }
{ }

void
state_chasing::update(boss& b)
{
	++state_tics_;

	b.chase_player();

	// rotate spikes

	const float da = .025f;
	for (auto& a : b.spike_angle) {
		a += da;
	}

	if (state_tics_ >= MIN_STATE_TICS) {
		if (rand()%16 == 0)
			b.set_state_aiming();
	}
}

//
//  a i m i n g
//

state_aiming::state_aiming(game& g)
: boss::state { g }
{ }

void
boss_aim(boss& b, const vec2f& target)
{
	const vec2f d = target - b.pos;
	const vec2f n = normalized(vec2f { -d.y, d.x });

	// rotate first spike towards player

	const float a = b.spike_angle[0];
	vec2f u { cosf(a), sinf(a) };
	b.spike_angle[0] -= .05f*dot(n, u);
}

void
boss_aim_transition(boss& b, float t)
{
	// speed

	b.speed = exp_tween<float>()(BOSS_SPEED, 0, t);

	// remaining spikes follow first spike

	const float da = 2.f*M_PI/boss::NUM_SPIKES;

	for (size_t i = 1; i <= boss::NUM_SPIKES/2; i++)
		b.spike_angle[i] = b.spike_angle[0] + exp_tween<float>()(i*da, 0, t);

	for (size_t i = 0; i < boss::NUM_SPIKES/2; i++)
		b.spike_angle[boss::NUM_SPIKES - 1 - i] = b.spike_angle[0] - exp_tween<float>()((i + 1)*da, 0, t);
}

void
state_aiming::update(boss& b)
{
	++state_tics_;

	b.chase_player();

	boss_aim(b, vec2f(game_.get_player_world_position()));
	boss_aim_transition(b, std::min(static_cast<float>(state_tics_)/AIMING_TICS, 1.f));

	if (state_tics_ >= AIMING_TICS)
		b.set_state_firing();
}

//
//  f i r i n g
//

state_firing::state_firing(game& g)
: boss::state { g }
{ }

void
state_firing::update(boss& b)
{
	++state_tics_;

	if (state_tics_%30 == 0) {
		const float a = b.spike_angle[0];
		const vec2f pos = b.pos + vec2f { cosf(a), sinf(a) }*b.radius*1.2f;
		vec2f dir = normalized(pos - b.pos);
		game_.add_foe(std::unique_ptr<foe>(new bullet { game_, pos, dir }));
	}

	// keep tracking player

	boss_aim(b, vec2f(game_.get_player_world_position()));
	for (size_t i = 1; i < boss::NUM_SPIKES; i++)
		b.spike_angle[i] = b.spike_angle[0];

	if (state_tics_ >= FIRING_TICS)
		b.set_state_post_firing();
}

//
//  p o s t _ f i r i n g
//

state_post_firing::state_post_firing(game& g)
: boss::state { g }
{ }

void
state_post_firing::update(boss& b)
{
	++state_tics_;

	b.chase_player();

	boss_aim_transition(b, 1.f - std::min(static_cast<float>(state_tics_)/POST_FIRING_TICS, 1.f));

	if (state_tics_ >= POST_FIRING_TICS)
		b.set_state_chasing();
}

//
//  b u l l e t
//

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

	vec2f pos_end = pos_ + dir_*static_cast<float>(LENGTH);

	if (std::min(pos_.x, pos_end.x) > grid_width ||
		std::max(pos_.x, pos_end.x) < 0 ||
		std::min(pos_.y, pos_end.y) > grid_height ||
		std::max(pos_.y, pos_end.y) < 0) {
		return false;
	}

	return true;
}

} // (anonymous namespace)

//
//  b o s s
//

boss::boss(game& g)
: phys_foe { g, vec2f { 100, 100 }, normalized(vec2f { 1.5f, .5f }), BOSS_SPEED, 30 }
{
	initialize_spikes();

	set_state_chasing();
}

void
boss::initialize_spikes()
{
	const float da = 2.f*M_PI/NUM_SPIKES;
	float a = 0;

	for (int i = 0; i < NUM_SPIKES; i++) {
		spike_angle[i] = a;
		a += da;
	}
}

void
boss::set_state_chasing()
{
	state_.reset(new state_chasing { game_ });
}

void
boss::set_state_aiming()
{
	state_.reset(new state_aiming { game_ });
}

void
boss::set_state_firing()
{
	state_.reset(new state_firing { game_ });
}

void
boss::set_state_post_firing()
{
	state_.reset(new state_post_firing { game_ });
}

bool
boss::update()
{
	move();

	state_->update(*this);

	return true;
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
	draw_core();
	draw_spikes();
}

void
boss::draw_core() const
{
	static const int NUM_SEGS = 13;

	glDisable(GL_TEXTURE_2D);
	glColor4f(1, 1, 0, 1);

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
	for (int i = 0; i < NUM_SPIKES; i++) {
		float a = spike_angle[i];

		glPushMatrix();

		glTranslatef(pos.x, pos.y, 0.f);
		glRotatef(a*180.f/M_PI - 90.f, 0, 0, 1);
		glTranslatef(0, radius*1.2f, 0.f);
		draw_spike();

		glPopMatrix();
	}
}

void
boss::draw_spike() const
{
	glColor4f(1, 1, 0, 1);

	glBegin(GL_LINE_LOOP);
	glVertex2i(-5, 0);
	glVertex2i(0, 15);
	glVertex2i(5, 0);
	glEnd();
}

boss::state::state(game& g)
: game_ { g }
, state_tics_ { 0 }
{ }
