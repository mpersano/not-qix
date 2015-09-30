#pragma once

#include <memory>
#include <vector>
#include <list>
#include <functional>

#include <ggl/noncopyable.h>
#include <ggl/vec2.h>
#include <ggl/vertex_array.h>

#include "widget.h"
#include "effect.h"
#include "entity.h"
#include "player.h"
#include "level.h"

class game;
class foe;

class game_state
{
public:
	game_state(game& g);
	virtual ~game_state() = default;

	virtual void draw() const = 0;
	virtual void update(unsigned dpad_state) = 0;

protected:
	game& game_;
};

class game : private ggl::noncopyable
{
public:
	game(int width, int height);

	void reset(const level *l);
	void update(unsigned dpad_state);
	void draw() const;

	unsigned get_cover_percent() const;

	vec2i get_player_screen_position() const;
	vec2i get_player_world_position() const;

	player& get_player();

	void reset_player(const vec2i& pos);

	void update_player(unsigned dpad_state);
	void update_entities();

	void draw_border() const;
	void draw_entities() const;
	void draw_player() const;

	void add_entity(std::unique_ptr<entity> f);
	void add_effect(std::unique_ptr<effect> e);

	void fill_grid(const std::vector<vec2i>& contour);
	void fill_grid(const vec2i& bottom_left, const vec2i& top_right);

	void enter_level_intro_state();
	void enter_select_initial_offset_state();
	void enter_select_initial_area_state();
	void enter_playing_state(const vec2i& bottom_left, const vec2i& top_right);
	void enter_level_completed_state();
	void enter_game_over_state();

	int operator()(int c, int r) const
	{ return grid[r*grid_cols + c]; }

	int viewport_width, viewport_height;

	std::vector<int> grid;
	int grid_rows, grid_cols;

	const level *cur_level;

	std::vector<vec2i> border;

	std::list<std::unique_ptr<entity>> entities;

	vec2i offset;

	int tics;

private:
	void set_state(std::unique_ptr<game_state> next_state);

	void draw_background() const;
	void draw_hud() const;
	void draw_effects() const;

	void update_hud();
	void update_effects();

	void update_border();
	void update_background();
	void update_cover_percent();

	void show_hud();
	void hide_hud();

	void add_boss();

	const foe *cur_boss_;

	player player_;
	unsigned cover_percent_;

	std::vector<std::unique_ptr<widget>> widgets_;
	std::vector<std::unique_ptr<effect>> effects_;

	ggl::vertex_array_texcoord<GLshort, 2, GLfloat, 2> background_filled_va_;
	ggl::vertex_array_texcoord<GLshort, 2, GLfloat, 2> background_unfilled_va_;
	ggl::vertex_array_flat<GLshort, 2> border_va_;

	std::unique_ptr<game_state> state_;
};
