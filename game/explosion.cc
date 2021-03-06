#include <cassert>

#include <ggl/gl.h>
#include <ggl/sprite.h>
#include <ggl/render.h>
#include <ggl/resources.h>

#include "util.h"
#include "bezier.h"
#include "explosion.h"

namespace {

const int TTL = 30;

static const struct explosion_info {
	int min_particles, max_particles;
	int min_fireballs, max_fireballs;
} explosion_infos[] = {
	{ 16, 30, 1, 3 },
	{ 70, 70, 5, 5 },
};

} // (anonymous namespace)

explosion::explosion(const vec2f& pos, int bang)
{
	assert(bang >= 0 && bang < sizeof explosion_infos/sizeof *explosion_infos);

	load_sprites();

	const auto& info = explosion_infos[bang];

	const int num_particles = rand<int>(info.min_particles, info.max_particles);
	for (size_t i = 0; i < num_particles; i++)
		particles_.emplace_back(particle_sprite_, pos);

	const int num_fireballs = rand<int>(info.min_fireballs, info.max_fireballs);
	for (size_t i = 0; i < num_fireballs; i++) {
		float a = rand<float>(0, 2.f*M_PI);
		float d = rand<float>(16.f, 64.f);
		vec2f p = pos + vec2f { sinf(a), cosf(a) }*d;

		const int frames = NUM_FLARE_FRAMES;
		flares_.emplace_back(flare_sprites_, frames, p, rand<float>(40, 64), 1.015f, rand<int>(30, 50), 2);
	}

	const int frames = NUM_RING_FRAMES;
	flares_.emplace_back(ring_sprites_, frames, pos, 60, 1.05f, 30, 1);
}

void
explosion::load_sprites()
{
	for (size_t i = 0; i < NUM_RING_FRAMES; i++) {
		char name[80];
		sprintf(name, "ring-%02d.png", i);
		ring_sprites_[i] = ggl::res::get_sprite(name);
	}

	for (size_t i = 0; i < NUM_FLARE_FRAMES; i++) {
		char name[80];
		sprintf(name, "explosion-%02d.png", i);
		flare_sprites_[i] = ggl::res::get_sprite(name);
	}

	particle_sprite_ = ggl::res::get_sprite("particle.png");
}

bool
explosion::do_update()
{
	bool rv = false;

	for (auto& p : particles_) {
		if (p.update())
			rv = true;
	}

	for (auto& p : flares_) {
		if (p.update())
			rv = true;
	}

	return rv;
}

void
explosion::draw() const
{
	for (auto& p : particles_)
		p.draw();

	ggl::render::set_color(ggl::white);

	for (auto& p : flares_)
		p.draw();
}

// particles

explosion::particle::particle(const ggl::sprite *sp, const vec2f& pos)
: sp_ { sp }
, pos_ { pos }
, dir_ { [&] { float a = rand<float>(0.f, 2.f*M_PI); return vec2f(cosf(a), sinf(a)); }() }
, tics_ { 0 }
, ttl_ { rand<int>(30, 50) }
, speed_ { rand<float>(3.2, 7.) }
{
	const bezier<rgb> gradient { { 1.f, .5f, 0.f }, { 1.f, 1.f, 0.f }, { 1.f, 1.f, 1.f } };
	color_ = gradient(rand<float>(0.f, 1.f));
}

bool
explosion::particle::update()
{
	if (tics_ >= ttl_)
		return false;

	++tics_;
	pos_ += dir_*speed_;
	speed_ *= .99f;

	return true;
}

void
explosion::particle::draw() const
{
	if (tics_ >= ttl_)
		return;

	// XXX
	const int w = 32;
	const int h = 10;

	vec2f up = vec2f { -dir_.y, dir_.x }*.5f*h;
	vec2f right = dir_*static_cast<float>(w);

	const vec2f p0 = pos_ + up;
	const vec2f p1 = pos_ - up;
	const vec2f p2 = pos_ - up + right;
	const vec2f p3 = pos_ + up + right;

	const float u0 = sp_->u0;
	const float u1 = sp_->u1;

	const float v0 = sp_->v0;
	const float v1 = sp_->v1;

	float a = 1.f - static_cast<float>(tics_)/ttl_;
	ggl::render::set_color({ color_.r, color_.g, color_.b, a });

	ggl::render::draw(sp_->tex, { { u0, v1 }, { u1, v0 } }, ggl::quad { p0, p1, p2, p3 }, 0);
}

// flares

explosion::flare::flare(const ggl::sprite **sprites, int frames, const vec2f& pos, float radius, float radius_factor, int ttl, float depth)
: sprites_ { sprites }
, frames_ { frames }
, pos_ { pos }
, angle_ { rand<float>(0.f, 2.f*M_PI) }
, radius_ { radius }
, radius_factor_ { radius_factor }
, tics_ { 0 }
, ttl_ { ttl }
, depth_ { depth }
{ }

bool
explosion::flare::update()
{
	if (tics_ >= ttl_)
		return false;

	++tics_;
	radius_ *= radius_factor_;
	angle_ += .13f;

	return true;
}

void
explosion::flare::draw() const
{
	if (tics_ >= ttl_)
		return;

	auto& s = sprites_[tics_*frames_/ttl_];

	ggl::render::push_matrix();
	ggl::render::translate(pos_);
	ggl::render::rotate(angle_);
	ggl::render::scale(radius_/s->width);
	s->draw(depth_);
	ggl::render::pop_matrix();
}
