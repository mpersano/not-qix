#pragma once

#include <memory>
#include <vector>
#include <list>
#include <functional>

#include <ggl/event.h>
#include <ggl/noncopyable.h>
#include <ggl/vec2.h>
#include <ggl/vertex_array.h>
#include <ggl/framebuffer.h>

#include "widget.h"
#include "effect.h"
#include "entity.h"
#include "player.h"
#include "post_filter.h"
#include "level.h"

namespace ggl {
class program;
}

class game;
class foe;

class game_state
{
public:
	game_state(game& g);
	virtual ~game_state() = default;

	virtual void update(unsigned dpad_state) = 0;
	virtual void draw() const = 0;
	virtual void draw_overlay() const = 0;

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

	player& get_player();

	vec2i get_player_screen_position() const;
	vec2i get_player_world_position() const;

	void reset_player(const vec2i& pos);

	bool update_player(unsigned dpad_state);

	void draw_player() const;

	void add_entity(std::unique_ptr<entity> f);
	void add_effect(std::unique_ptr<effect> e);
	void add_post_filter(std::unique_ptr<dynamic_post_filter> f);

	void start_screenshake(int duration, float intensity);
	void start_screenflash(int duration);

	void fill_grid(const std::vector<vec2i>& contour);
	void fill_grid(const vec2i& bottom_left, const vec2i& top_right);

	void enter_level_intro_state();
	void enter_select_initial_offset_state();
	void enter_select_initial_area_state();
	void enter_playing_state(const vec2i& bottom_left, const vec2i& top_right);
	void enter_level_completed_state();
	void enter_game_over_state();

	using cover_update_event_handler = std::function<void(unsigned)>;
	ggl::connectable_event<cover_update_event_handler>& get_cover_update_event();

	using start_event_handler = std::function<void(void)>;
	ggl::connectable_event<start_event_handler>& get_start_event();

	using stop_event_handler = std::function<void(void)>;
	ggl::connectable_event<stop_event_handler>& get_stop_event();

	int operator()(int c, int r) const
	{ return grid[r*grid_cols + c]; }

	vec2f get_viewport_offset() const;

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

	void draw_scene() const;
	void draw_background() const;

	void update_border();
	void update_background();
	void update_cover_percent();

	vec2f find_foe_pos(int radius) const;
	void add_foes();

	const foe *cur_boss_;

	player player_;
	unsigned cover_percent_;

	int shake_tics_, shake_ttl_;
	float shake_intensity_;
	vec2f shake_dir_;

	int flash_tics_, flash_ttl_;
	const ggl::program *flash_program_;

	std::vector<std::unique_ptr<widget>> widgets_;
	std::vector<std::unique_ptr<effect>> effects_;
	std::vector<std::unique_ptr<dynamic_post_filter>> post_filters_;

	ggl::vertex_array_texcoord<GLshort, 2, GLfloat, 2> background_filled_va_;
	ggl::vertex_array_texcoord<GLshort, 2, GLfloat, 2> background_unfilled_va_;
	ggl::vertex_array_texcoord<GLshort, 2, GLshort, 2> border_va_;
	const ggl::texture *border_texture_;

	std::unique_ptr<game_state> state_;

	ggl::event<cover_update_event_handler> cover_update_event_;
	ggl::event<start_event_handler> start_event_;
	ggl::event<stop_event_handler> stop_event_;

	ggl::framebuffer render_target_0_, render_target_1_;

	passthru_filter passthru_filter_;
};
