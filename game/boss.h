#pragma once

#include <memory>

#include "foe.h"

class boss : public phys_foe
{
public:
	boss(game& g);

	void draw() const override;
	bool update() override;

	bool is_boss() const override
	{ return true; }

	class state {
	public:
		state(game& g);
		virtual ~state() = default;

		virtual void update(boss& b) = 0;

	protected:
		game& game_;
		int state_tics_;
	};

	static const int NUM_SPIKES = 7;
	float spike_angle[NUM_SPIKES];

	void set_state_chasing();
	void set_state_aiming();
	void set_state_firing();
	void set_state_post_firing();

	void chase_player();

private:
	void initialize_spikes();

	void draw_core() const;
	void draw_spikes() const;
	void draw_spike() const;

	std::unique_ptr<state> state_;
};
