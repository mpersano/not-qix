#pragma once

#include "foe.h"

class powerup : public foe
{
public:
	powerup(game& g, const vec2f& pos, const vec2f& dir);

	void draw() const override;
	bool update() override;

	bool is_boss() const
	{ return false; }

	bool intersects(const vec2i&, const vec2i&) const
	{ return false; }

private:
	bool update_moving();
	bool update_sliding();

	enum state { MOVING, SLIDING } state_;
	vec2f pos_;
	vec2f dir_;
};
