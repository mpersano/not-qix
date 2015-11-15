#pragma once

#include <vector>

#include <ggl/event.h>
#include <ggl/noncopyable.h>
#include <ggl/vec2.h>

class game;

namespace ggl {
class sprite;
class sprite_batch;
};

class player : private ggl::noncopyable
{
public:
	player(game& g);

	void reset(const vec2i& pos);
	bool update(unsigned dpad_state);

	void draw(ggl::sprite_batch& sb) const;
	const vec2i get_position() const;

	vec2i get_grid_position() const;
	void set_grid_position(const vec2i& p);

	enum class direction { UP, DOWN, LEFT, RIGHT, NONE };

	using respawn_event_handler = std::function<void(int)>;
	ggl::connectable_event<respawn_event_handler>& get_respawn_event();

	using death_event_handler = std::function<void()>;
	ggl::connectable_event<death_event_handler>& get_death_event();

private:
	void respawn(const vec2i& pos);

	void draw_trail(int start_index) const;
	void draw_head(ggl::sprite_batch& sb) const;

	void move_slide(direction dir);
	void move_extend(direction dir);
	void check_foe_collisions();
	void die();

	void update_idle(unsigned dpad_state);
	void update_sliding(unsigned dpad_state);
	void update_extending_idle(unsigned dpad_state);
	void update_extending(unsigned dpad_state);
	void update_exploding(unsigned dpad_state);
	void update_death(unsigned dpad_state);

	enum class state { IDLE, SLIDING, EXTENDING_IDLE, EXTENDING, EXPLODING, DEATH, DEAD };
	void set_state(state next_state);

	game& game_;
	vec2i pos_, next_pos_;
	std::vector<vec2i> extend_trail_;
	state state_;
	int state_tics_;
	int lives_left_;

	static const int NUM_FRAMES = 64;
	const ggl::sprite *sprites_core_[NUM_FRAMES];
	const ggl::sprite *sprites_shield_[NUM_FRAMES];

	ggl::event<respawn_event_handler> respawn_event_;
	ggl::event<death_event_handler> death_event_;
};
