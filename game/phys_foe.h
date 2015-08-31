#pragma once

#include "foe.h"

class phys_foe : public foe
{
public:
	phys_foe(game& g, const vec2f& pos, const vec2f& dir, float speed, float radius);

	bool intersects(const vec2i& from, const vec2i& to) const override;

	void update_position();

	vec2f get_position() const;

protected:
	vec2f pos_;
	vec2f dir_;
	float speed_;
	float radius_;
};
