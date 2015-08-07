#pragma once

#include <vector>
#include <memory>

#include "common.h"
#include "vec2.h"

enum class direction { UP, DOWN, LEFT, RIGHT };

class game;
class level;

class player
{
public:
	player(game& g);

	void reset();
	void move(direction dir, bool button_pressed);
	void update(float dt);
	void draw() const;

private:
	void move_slide(direction dir);
	void move_extend(direction dir);

	enum class state { IDLE, SLIDING, EXTENDING_IDLE, EXTENDING };

	const vec2<int> get_position() const;
	void set_state(state next_state);

	game& game_;
	vec2<int> pos_, next_pos_;
	std::vector<vec2<int>> extend_trail_;
	state state_;
	float state_t_;
};

class foe
{
public:
	foe(game& g);
	virtual ~foe() { }

	virtual bool update(float dt) = 0;
	virtual void draw() const = 0;

protected:
	game& game_;
};

class game
{
public:
	game(int width, int height);

	void reset(const level *l);
	void move(direction dir, bool button_pressed);
	void update(float dt);
	void draw() const;
	void fill_grid(const std::vector<vec2<int>>& contour);

	bool grid[GRID_ROWS*GRID_COLS];
	int cell_size;

private:
	void draw_background() const;
	void draw_border() const;

	int base_x_, base_y_;
	player player_;
	std::vector<std::unique_ptr<foe>> foes_;
	const level *cur_level_;
	float elapsed_t_;
};
