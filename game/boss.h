#pragma once

#include "script_interface.h"
#include "phys_foe.h"

namespace ggl {
class sprite;
}

class boss : public phys_foe
{
public:
	boss(game& g);

	void draw() const override;
	bool update() override;

	bool is_boss() const override
	{ return true; }

	void on_miniboss_killed();

private:
	enum class state { CHASING, PRE_FIRING, FIRING, POST_FIRING } state_;

	void set_state(state next_state);

	void update_chasing();
	void update_pre_firing();
	void update_firing();
	void update_post_firing();

	void chase_player();
	void aim_player();

	void draw_core() const;
	void draw_spikes() const;
	void draw_spike(float a) const;

	int state_tics_;
	static const int NUM_SPIKES = 7;
	float spike_angle_;
	int miniboss_spawned_;

	static const int MIN_CHASE_TICS = 360;
	static const int PRE_FIRING_TICS = 90;
	static const int FIRING_TICS = 180;
	static const int POST_FIRING_TICS = 90;

	const ggl::sprite *core_sprite_;
	const ggl::sprite *spike_sprite_;
	std::unique_ptr<script_thread> script_thread_;
};
