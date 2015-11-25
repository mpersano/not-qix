#pragma once

#include <ggl/vec2.h>

#include "rgb.h"
#include "effect.h"

namespace ggl {
class sprite;
};

class explosion : public effect
{
public:
	explosion(const vec2f& pos, float bang);

	void draw() const override;

	bool is_position_absolute() const override
	{ return false; }

private:
	bool do_update() override;

	void load_sprites();

	static const int NUM_RING_FRAMES = 16;
	const ggl::sprite *ring_sprites_[NUM_RING_FRAMES];

	static const int NUM_FLARE_FRAMES = 16;
	const ggl::sprite *flare_sprites_[NUM_FLARE_FRAMES];

	const ggl::sprite *particle_sprite_;

	class flare
	{
	public:
		flare(const ggl::sprite **sprites, int frames, const vec2f& pos, float radius, float radius_factor, int ttl, float depth);

		bool update();
		void draw() const;

	private:
		const ggl::sprite **sprites_;
		int frames_;
		vec2f pos_;
		float angle_;
		float radius_, radius_factor_;
		int tics_, ttl_;
		float depth_;
	};

	class particle
	{
	public:
		particle(const ggl::sprite *sp, const vec2f& pos);

		bool update();
		void draw() const;

	private:
		const ggl::sprite *sp_;
		vec2f pos_, dir_;
		int tics_, ttl_;
		float speed_;
		rgb color_;
	};

	std::vector<particle> particles_;
	std::vector<flare> flares_;
};
