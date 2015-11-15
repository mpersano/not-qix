#pragma once

#include <ggl/noncopyable.h>
#include <ggl/vec2.h>

class game;

namespace ggl {
class sprite_batch;
}

class entity : private ggl::noncopyable
{
public:
	entity(game& g);
	virtual ~entity() = default;

	virtual bool update() = 0;
	virtual void draw(ggl::sprite_batch& sb) const = 0;

	virtual bool intersects(const vec2i& from, const vec2i& to) const = 0;
	virtual bool intersects(const vec2i& center, float radius) const = 0;

protected:
	game& game_;
};
