#pragma once

#include <vector>
#include <memory>

#include <ggl/vec2.h>
#include <ggl/vertex_array.h>

#include "common.h"

namespace ggl {
class font;
}

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

	const vec2i get_position() const;
	void set_state(state next_state);

	static const int PLAYER_RADIUS = 5;

	game& game_;
	vec2i pos_, next_pos_;
	std::vector<vec2i> extend_trail_;
	state state_;
	float state_t_;
};

class game
{
public:
	game(int width, int height);

	void reset(const level *l);
	void move(direction dir, bool button_pressed);
	void update(float dt);
	void draw() const;
	void fill_grid(const std::vector<vec2i>& contour);

	bool grid[GRID_ROWS*GRID_COLS];
	int cell_size;

private:
	void draw_background() const;
	void draw_border() const;

	void initialize_vas();
	void initialize_background_vas();
	void initialize_border_va();

	int base_x_, base_y_;
	player player_;
	const level *cur_level_;
	float elapsed_t_;

	ggl::vertex_array_texcoord<GLshort, 2, GLfloat, 2> background_filled_va_;
	ggl::vertex_array_texcoord<GLshort, 2, GLfloat, 2> background_unfilled_va_;
	ggl::vertex_array_flat<GLshort, 2> border_va_;
	const ggl::font *font_;
};
