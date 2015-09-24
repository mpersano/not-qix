#pragma once

#include "entity.h"

class foe : public entity
{
public:
	foe(game& g, const vec2f& pos, float radius);

	bool intersects(const vec2i& from, const vec2i& to) const override;
	bool intersects(const vec2i& center, float radius) const override;

	void set_direction(const vec2f& dir);
	void set_speed(float speed);

	void update_position();
	void rotate_to_player();

	vec2f get_position() const;

private:
	bool collide_against_edge(const vec2f& v0, const vec2f& v1);

protected:
	vec2f pos_;
	vec2f dir_;
	float speed_;
	float radius_;
};
