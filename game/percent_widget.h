#pragma once

#include <ggl/event.h>

#include "widget.h"

namespace ggl {
class font;
}

class game;

class percent_widget : public widget
{
public:
	percent_widget(game& g);

	bool update() override;
	void draw() const override;

private:
	void on_cover_update(unsigned percent);
	void on_game_start();
	void on_game_stop();

	int get_base_x() const;
	int get_base_y() const;

	unsigned get_value() const;
	float get_text_scale() const;

	void draw_frame() const;
	void draw_digits() const;
	void draw_char(const ggl::font *f, wchar_t ch, int base_x, int base_y) const;

	enum position { LEFT, RIGHT } position_;
	unsigned cur_value_, next_value_;

	enum state { HIDDEN, INTRO, IDLE, OUTRO } state_;
	void set_state(state next_state);
	int state_tics_;

	bool updating_;
	int update_tics_, update_ttl_;

	bool hidden_;

	const ggl::font *large_font_, *small_font_;
	const ggl::sprite *frame_;

	ggl::event_connection_ptr cover_update_conn_;
	ggl::event_connection_ptr game_start_conn_;
	ggl::event_connection_ptr game_stop_conn_;
	ggl::event_connection_ptr effect_finished_conn_;
};
