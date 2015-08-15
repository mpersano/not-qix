#pragma once

#include <ggl/noncopyable.h>
#include <ggl/vec2.h>

class game;

class foe : private ggl::noncopyable
{
public:
	foe(game& g);
	virtual ~foe() = default;

	virtual void draw() const = 0;
	virtual bool update() = 0;

	virtual bool is_boss() const = 0;

protected:
	game& game_;
};

class phys_foe : public foe
{
public:
	phys_foe(game& g, vec2f pos, vec2f dir, float speed, float radius);

	void move();

	vec2f pos;
	vec2f dir;
	float speed;
	float radius;
};
