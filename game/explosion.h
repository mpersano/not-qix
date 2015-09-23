#pragma once

#include <ggl/vec2.h>

#include "effect.h"

class explosion : public effect
{
public:
	explosion(const vec2f& pos);

	bool update() override;
	void draw() const override;

	bool is_position_absolute() const override
	{ return false; }

private:
	vec2f pos_;
	float radius_;
	int tics_;
};
