#pragma once

#include <ggl/event.h>

#include "quad.h"
#include "widget.h"

namespace ggl {
class sprite;
}

class lives_widget : public widget
{
public:
	lives_widget(game& g);

	bool update() override;
	void draw() const override;

private:
	void initialize_text(int lives_left);

	void on_player_respawn(int lives_left);
	void on_player_death();

	int show_tics_;

	std::unique_ptr<quad> text_;

	const ggl::sprite *circle_sprite_;
	const ggl::texture *shine_texture_;

	ggl::event_connection_ptr respawn_conn_;
	ggl::event_connection_ptr death_conn_;
};
