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

	virtual bool intersects(const vec2i& from, const vec2i& to) const = 0;

protected:
	game& game_;
};
