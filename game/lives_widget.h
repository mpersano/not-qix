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
	void draw(ggl::sprite_batch& sb) const override;

private:
	void initialize_text(int lives_left);

	void on_player_respawn(int lives_left);
	void on_player_death();

	bool hide_;

	enum state { HIDDEN, INTRO, IDLE, OUTRO } state_;
	void set_state(state next_state);
	int state_tics_;

	std::wstring text_;
	const ggl::font *font_;

	std::unique_ptr<quad> circle_;

	ggl::event_connection_ptr respawn_conn_;
	ggl::event_connection_ptr death_conn_;
};
