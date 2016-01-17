#pragma once

#include <memory>
#include <tuple>

#include <ggl/app.h>

#include "app_state.h"

class game_app : public ggl::app
{
public:
	game_app();

	void init(int viewport_width, int viewport_height) override;
	void update_and_render(float dt) override;

	int get_scene_width() const
	{ return scene_width_; }

	int get_scene_height() const
	{ return scene_height_; }

	int get_viewport_width() const
	{ return viewport_width_; }

	int get_viewport_height() const
	{ return viewport_height_; }

	void start_in_game();

private:
	void init_states();
	void init_gl_state();

	std::unique_ptr<app_state> level_selection_state_;
	std::unique_ptr<app_state> in_game_state_;
	std::unique_ptr<app_state> transition_state_;

	app_state *cur_state_;

	float update_t_;

	int viewport_width_, viewport_height_;
	int scene_width_, scene_height_;
};
