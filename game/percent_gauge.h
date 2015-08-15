#pragma once

#include "effect.h"

namespace ggl {
class font;
}

class game;

class percent_gauge : public effect
{
public:
	percent_gauge(game& g, int viewport_height);

	bool update(float dt) override;
	void draw() const override;

private:
	int get_base_x() const;
	int get_base_y() const;

	unsigned get_value() const;

	void draw_frame() const;
	void draw_digits() const;
	void draw_char(const ggl::font *f, wchar_t ch, int base_x, int base_y) const;

	enum {
		TOP_MARGIN = 20,
		LEFT_MARGIN = 20,
		WIDTH = 160,
		HEIGHT = 60,
	};

	game& game_;
	int viewport_height_;
	bool position_top_;
	unsigned cur_value_, next_value_;

	enum state { INTRO, IDLE, OUTRO } state_;
	void set_state(state next_state);
	float state_t_;

	bool updating_;
	float update_t_;

	const ggl::font *large_font_, *small_font_;
};
