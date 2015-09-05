#pragma once

#include <vector>
#include <list>

#include <ggl/noncopyable.h>
#include <ggl/vec2.h>
#include <ggl/vertex_array.h>

#include "foe.h"
#include "player.h"
#include "level.h"

class game : private ggl::noncopyable
{
public:
	game(int width, int height);

	void reset(const level *l);

	void update(unsigned dpad_state);
	void draw() const;
	void fill_grid(const std::vector<vec2i>& contour);

	unsigned get_cover_percent() const;

	vec2i get_player_screen_position() const;
	vec2i get_player_world_position() const;

	void add_foe(std::unique_ptr<foe> f);

	void set_player_grid_position(const vec2i& p);

	std::vector<int> grid;
	int grid_rows, grid_cols;
	std::vector<vec2i> border;
	std::list<std::unique_ptr<foe>> foes;

private:
	enum class state { SELECTING_START_AREA, PLAYING };

	vec2f get_offset() const;

	void draw_selecting_start_area() const;
	void draw_playing() const;

	void update_selecting_offset(unsigned dpad_state);
	void update_selecting_start_area(unsigned dpad_state);
	void update_playing(unsigned dpad_state);

	void draw_background() const;
	void draw_border() const;
	void draw_foes() const;

	void initialize_border();
	void initialize_background();
	void update_cover_percent();

	void reset_start_area();
	void start_playing();

	int viewport_width_, viewport_height_;
	vec2i offset_, next_offset_;
	player player_;
	const level *cur_level_;
	bool scrolling_;
	float scroll_tics_;

	static const int SCROLL_TICS = 30;

	unsigned cover_percent_;

	state state_;

	std::pair<vec2i, vec2i> start_area_;
	int start_area_tics_;
	unsigned prev_dpad_state_;

	ggl::vertex_array_texcoord<GLshort, 2, GLfloat, 2> background_filled_va_;
	ggl::vertex_array_texcoord<GLshort, 2, GLfloat, 2> background_unfilled_va_;
	ggl::vertex_array_flat<GLshort, 2> border_va_;
};
