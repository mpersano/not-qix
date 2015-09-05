#pragma once

#include "script_interface.h"
#include "phys_foe.h"

namespace ggl {
class sprite;
}

class boss : public phys_foe
{
public:
	boss(game& g, const vec2f& pos);

	void draw() const override;
	bool update() override;

	bool is_boss() const override
	{ return true; }

	void on_miniboss_killed();

	void rotate_spike_to_player();
	void rotate_spike(float a);
	void set_spike_dispersion(float t);
	void fire_bullet();

private:
	void draw_core() const;
	void draw_spikes() const;
	void draw_spike(float a) const;

	static const int NUM_SPIKES = 7;
	float spike_angle_;
	float spike_dispersion_; // 0 to 1

	int miniboss_spawned_;

	const ggl::sprite *core_sprite_;
	const ggl::sprite *spike_sprite_;
	std::unique_ptr<script_thread> script_thread_;
};
