#pragma once

#include <vector>

#include <ggl/noncopyable.h>
#include <ggl/vec2.h>

enum class direction { UP, DOWN, LEFT, RIGHT };

class game;

class player : private ggl::noncopyable
{
public:
	player(game& g);

	void reset(const vec2i& pos);
	void move(direction dir, bool button);
	void update();

	void draw() const;
	const vec2i get_position() const;

	vec2i get_grid_position() const;
	void set_grid_position(const vec2i& p);

private:
	void die();

	void move_slide(direction dir);
	void move_extend(direction dir);

	enum class state { IDLE, SLIDING, EXTENDING_IDLE, EXTENDING };

	void set_state(state next_state);

	void check_foe_collisions();

	static const int SLIDE_TICS = 3;

	game& game_;
	vec2i pos_, next_pos_;
	std::vector<vec2i> extend_trail_;
	state state_;
	int state_tics_;
};
