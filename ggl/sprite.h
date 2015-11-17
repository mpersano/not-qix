#pragma once

#include <ggl/align.h>
#include <ggl/vec2.h>
#include <ggl/rgba.h>

namespace ggl {

class texture;
class sprite_batch;

class sprite
{
public:
	sprite(const texture *tex, int u, int v, int width, int height);

	void draw(sprite_batch& sb, float depth) const;
	void draw(sprite_batch& sb, float depth, const vec2f& pos) const;
	void draw(sprite_batch& sb, float depth, const vec2f& pos, vert_align va, horiz_align ha) const;

	const texture *tex;
	int width, height;
	float u0, u1, v0, v1;
};

}
