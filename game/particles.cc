#include <ggl/resources.h>
#include <ggl/sprite.h>
#include <ggl/render.h>

#include "bezier.h"
#include "util.h"
#include "particles.h"

particles::particles(const vec2f& pos, int num_particles, const gradient& g)
{
	particles_.reserve(num_particles);

	for (size_t i = 0; i < num_particles; i++)
		particles_.emplace_back(pos, g);
}

void
particles::draw() const
{
	for (auto& p : particles_)
		p.draw();
}

bool
particles::do_update()
{
	bool rv = false;

	for (auto& p : particles_) {
		if (p.update())
			rv = true;
	}

	return rv;
}

particles::particle::particle(const vec2f& origin, const gradient& g)
: sprite_ { ggl::res::get_sprite("star.png") }
, pos_ { origin }
, speed_ { [&] { const float a = rand<float>(0, 2.f*M_PI); return .7f*rand<float>(3., 5.)*vec2f(cosf(a), sinf(a)); }() }
, angle_ { rand<float>(0, 2.f*M_PI) }
, angle_speed_ { rand<float>(-.15, .15) }
, tics_ { 0 }
, ttl_ { rand<int>(20, 50) }
, color_ { g(rand<float>(0, 1)) }
{ }

void
particles::particle::draw() const
{
	if (tics_ >= ttl_)
		return;

	const int ft = .8f*ttl_;
	const float a = tics_ < ft ? 1.f : 1.f - static_cast<float>(tics_ - ft)/(ttl_ - ft);

	ggl::render::set_color({ color_.r, color_.g, color_.b, a });

	ggl::render::push_matrix();
	ggl::render::translate(pos_);
	ggl::render::rotate(angle_);
	sprite_->draw(0);
	ggl::render::pop_matrix();
}

bool
particles::particle::update()
{
	if (tics_ >= ttl_)
		return false;

	++tics_;
	pos_ += speed_;
	speed_ += vec2f(0, -.15f);
	angle_ += angle_speed_;

	return true;
}
