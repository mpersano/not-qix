#pragma once

#include <memory>
#include <vector>
#include <list>
#include <functional>

#include <ggl/noncopyable.h>
#include <ggl/vec2.h>
#include <ggl/vertex_array.h>
#include <ggl/event.h>

#include "foe.h"
#include "player.h"
#include "level.h"

class game;

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

	using game_started_event_handler = std::function<void(void)>;
	ggl::connectable_event<game_started_event_handler>& get_game_started_event();

	using cover_changed_event_handler = std::function<void(unsigned)>;
	ggl::connectable_event<cover_changed_event_handler>& get_cover_changed_event();

	unsigned get_cover_percent() const;

	vec2i get_player_screen_position() const;
	vec2i get_player_world_position() const;

	void reset_player(const vec2i& pos);

	void update_player(unsigned dpad_state);
	void update_foes();

	void draw_border() const;
	void draw_foes() const;
	void draw_player() const;

	void add_boss();
	void add_foe(std::unique_ptr<foe> f);

	void fill_grid(const std::vector<vec2i>& contour);
	void fill_grid(const vec2i& bottom_left, const vec2i& top_right);

	void enter_level_intro_state();
	void enter_select_initial_offset_state();
	void enter_select_initial_area_state();
	void enter_playing_state(const vec2i& bottom_left, const vec2i& top_right);

	int viewport_width, viewport_height;

	std::vector<int> grid;
	int grid_rows, grid_cols;

	const level *cur_level;

	std::vector<vec2i> border;

	std::list<std::unique_ptr<foe>> foes;

	vec2i offset;

private:
	void set_state(std::unique_ptr<game_state> next_state);

	void draw_background() const;

	void update_border();
	void update_background();
	void update_cover_percent();

	player player_;
	unsigned cover_percent_;

	ggl::vertex_array_texcoord<GLshort, 2, GLfloat, 2> background_filled_va_;
	ggl::vertex_array_texcoord<GLshort, 2, GLfloat, 2> background_unfilled_va_;
	ggl::vertex_array_flat<GLshort, 2> border_va_;

	std::unique_ptr<game_state> state_;

	ggl::event<game_started_event_handler> game_started_event_;
	ggl::event<cover_changed_event_handler> cover_changed_event_;
};
