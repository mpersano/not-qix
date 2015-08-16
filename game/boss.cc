#include <cmath>

#include <ggl/texture.h>
#include <ggl/sprite.h>
#include <ggl/resources.h>

#include "tween.h"
#include "game.h"
#include "boss.h"

namespace {

static const float BOSS_SPEED = 2;
static const float BOSS_RADIUS = 30;
static const float SPIKE_RADIUS = 36;

void
draw_sprite(const ggl::sprite *sprite, float x, float y)
{
	const int w = sprite->width;
	const int h = sprite->height;

	const float x0 = x;
	const float x1 = x + w;

	const float y0 = y;
	const float y1 = y + h;

	const float u0 = sprite->u0;
	const float u1 = sprite->u1;

	const float v0 = sprite->v0;
	const float v1 = sprite->v1;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);

	glColor4f(1, 1, 1, 1);

	sprite->tex->bind();

	(ggl::vertex_array_texcoord<GLfloat, 2, GLfloat, 2>
		{ { x0, y0, u0, v1 },
		  { x1, y0, u1, v1 },
		  { x0, y1, u0, v0 },
		  { x1, y1, u1, v0 } }).draw(GL_TRIANGLE_STRIP);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

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
	vec2f dir_;
	const ggl::sprite *sprite_;
};

bullet::bullet(game& g, const vec2f& pos, const vec2f& dir)
: foe { g }
, pos_ { pos }
, dir_ { dir }
, sprite_ { ggl::res::get_sprite("bullet.png") }
{ }

void
bullet::draw() const
{
#if 0
	glColor4f(1, 0, 0, 1);

	glBegin(GL_LINES);
	glVertex2f(pos_.x, pos_.y);
	glVertex2f(pos_.x + LENGTH*dir_.x, pos_.y + LENGTH*dir_.y);
	glEnd();
#else
	const int w = sprite_->width;
	const int h = sprite_->height;

	vec2f up = vec2f { -dir_.y, dir_.x }*.5f*h;
	vec2f right = dir_*static_cast<float>(w);

	const vec2f p0 = pos_ + up;
	const vec2f p1 = pos_ + up + right;
	const vec2f p2 = pos_ - up;
	const vec2f p3 = pos_ - up + right;

	const float u0 = sprite_->u0;
	const float u1 = sprite_->u1;

	const float v0 = sprite_->v0;
	const float v1 = sprite_->v1;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);

	glColor4f(1, 1, 1, 1);

	sprite_->tex->bind();

	(ggl::vertex_array_texcoord<GLfloat, 2, GLfloat, 2>
		{ { p0.x, p0.y, u0, v1 },
		  { p1.x, p1.y, u1, v1 },
		  { p2.x, p2.y, u0, v0 },
		  { p3.x, p3.y, u1, v0 } }).draw(GL_TRIANGLE_STRIP);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

#endif
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
: phys_foe { g, vec2f { 100, 100 }, normalized(vec2f { 1.5f, .5f }), BOSS_SPEED, BOSS_RADIUS }
, spike_angle_ { 0 }
, core_sprite_ { ggl::res::get_sprite("boss-core.png") }
, spike_sprite_ { ggl::res::get_sprite("boss-spike.png") }
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

	speed_ = exp_tween<float>()(BOSS_SPEED, 0, static_cast<float>(state_tics_)/PRE_FIRING_TICS);

	if (state_tics_ >= PRE_FIRING_TICS)
		set_state(state::FIRING);
}

void
boss::update_firing()
{
	if (state_tics_%30 == 0) {
		const float a = spike_angle_;
		const vec2f p = pos_ + vec2f { cosf(a), sinf(a) }*SPIKE_RADIUS;
		vec2f d = normalized(p - pos_);
		game_.add_foe(std::unique_ptr<foe>(new bullet { game_, p, d }));
	}

	aim_player();

	if (state_tics_ >= FIRING_TICS)
		set_state(state::POST_FIRING);
}

void
boss::update_post_firing()
{
	chase_player();

	speed_ = exp_tween<float>()(BOSS_SPEED, 0, 1.f - static_cast<float>(state_tics_)/POST_FIRING_TICS);

	if (state_tics_ >= POST_FIRING_TICS)
		set_state(state::CHASING);
}

void
boss::aim_player()
{
	const vec2f d = vec2f(game_.get_player_world_position()) - pos_;
	const vec2f n = normalized(vec2f { -d.y, d.x });

	// rotate spike towards player

	const float a = spike_angle_;
	vec2f u { cosf(a), sinf(a) };
	spike_angle_ -= .05f*dot(n, u);
}

void
boss::chase_player()
{
	vec2f n { -dir_.y, dir_.x };

	float da = .025f*dot(n, normalized(vec2f(game_.get_player_world_position()) - pos_));

	const float c = cosf(da);
	const float s = sinf(da);

	vec2f next_dir { dot(dir_, vec2f { c, -s }), dot(dir_, vec2f { s, c }) };
	dir_ = next_dir;
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
	draw_sprite(core_sprite_, pos_.x - .5f*core_sprite_->width, pos_.y - .5f*core_sprite_->height);

#if 0
	static const int NUM_SEGS = 13;

	float a = 0;
	const float da = 2.f*M_PI/NUM_SEGS;

	glBegin(GL_LINE_LOOP);

	for (int i = 0; i < NUM_SEGS; i++) {
		vec2f p = pos_ + vec2f { cosf(a), sinf(a) }*radius_;
		glVertex2f(p.x, p.y);
		a += da;
	}

	glEnd();
#endif
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

	glTranslatef(pos_.x, pos_.y, 0.f);
	glRotatef(a*180.f/M_PI - 90.f, 0, 0, 1);
	glTranslatef(0, SPIKE_RADIUS, 0.f);

	draw_sprite(spike_sprite_, -.5f*spike_sprite_->width, 0);

#if 0
	glBegin(GL_LINE_LOOP);
	glVertex2i(-5, 0);
	glVertex2i(0, 15);
	glVertex2i(5, 0);
	glEnd();
#endif

	glPopMatrix();
}
