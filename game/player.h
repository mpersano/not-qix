#pragma once

#include <vector>

#include <ggl/noncopyable.h>
#include <ggl/vec2.h>

enum class direction { UP, DOWN, LEFT, RIGHT };

class game;

namespace ggl {
class sprite;
};

class player : private ggl::noncopyable
{
public:
	player(game& g);

	void reset(const vec2i& pos);
	void update(unsigned dpad_state);

	void draw() const;
	const vec2i get_position() const;

	vec2i get_grid_position() const;
	void set_grid_position(const vec2i& p);

private:
	void die();

	void move(direction dir, bool button);
	void move_slide(direction dir);
	void move_extend(direction dir);

	enum class state { IDLE, SLIDING, EXTENDING_IDLE, EXTENDING };

	void set_state(state next_state);

	void check_foe_collisions();

	static const int SLIDE_TICS = 3;
	static const int NUM_FRAMES = 64;

	game& game_;
	vec2i pos_, next_pos_;
	std::vector<vec2i> extend_trail_;
	state state_;
	int state_tics_;
	int tic_;

	const ggl::sprite *sprites_core_[NUM_FRAMES];
	const ggl::sprite *sprites_shield_[NUM_FRAMES];
};
