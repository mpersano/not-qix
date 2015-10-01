#pragma once

#include <ggl/event.h>

#include "widget.h"

class lives_widget : public widget
{
public:
	lives_widget(game& g);

	bool update() override;
	void draw() const override;

private:
	void on_player_respawn();
	void on_player_death();

	int show_tics_;
	ggl::event_connection_ptr respawn_conn_;
	ggl::event_connection_ptr death_conn_;
};
