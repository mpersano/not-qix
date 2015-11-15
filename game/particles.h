#pragma once

#include <ggl/rgba.h>
#include <ggl/vec2.h>

#include "rgb.h"
#include "bezier.h"
#include "effect.h"

namespace ggl {
class sprite;
};

using gradient = bezier<rgb>;

class particles : public effect
{
public:
	particles(const vec2f& pos, int num_particles, const gradient& g);

	void draw(ggl::sprite_batch& sb) const override;

	bool is_position_absolute() const override
	{ return true; }

private:
	bool do_update() override;

	class particle {
	public:
		particle(const vec2f& origin, const gradient& g);

		void draw(ggl::sprite_batch& sb) const;
		bool update();

	private:
		const ggl::sprite *sprite_;
		vec2f pos_, speed_;
		float angle_, angle_speed_;
		int tics_, ttl_;
		rgb color_;
	};

	std::vector<particle> particles_;
	ggl::sprite *sprite_;
};
