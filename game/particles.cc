#include <ggl/resources.h>
#include <ggl/sprite_batch.h>

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
particles::draw(ggl::sprite_batch& sb) const
{
	for (auto& p : particles_)
		p.draw(sb);
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
: sprite { ggl::res::get_sprite("star.png") }
{
	tics = 0;
	ttl = rand<int>(20, 50);
	pos = origin;

	const float a = rand<float>(0, 2.f*M_PI);
	speed = .7f*rand<float>(3., 5.)*vec2f(cosf(a), sinf(a));

	angle = rand<float>(0, 2.f*M_PI);
	angle_speed = rand<float>(-.15, .15);

	color = g(rand<float>(0, 1));
}

void
particles::particle::draw(ggl::sprite_batch& sb) const
{
	if (tics >= ttl)
		return;

	const int ft = .8f*ttl;
	const float a = tics < ft ? 1.f : 1.f - static_cast<float>(tics - ft)/(ttl - ft);

	sb.set_color({ color.r, color.g, color.b, a });

	sb.push_matrix();
	sb.translate(pos);
	sb.rotate(angle);

	sprite->draw(sb, 0);

	sb.pop_matrix();
}

bool
particles::particle::update()
{
	if (tics >= ttl)
		return false;

	pos += speed;
	speed += vec2f(0, -.15f);
	angle += angle_speed;

	++tics;

	return true;
}
