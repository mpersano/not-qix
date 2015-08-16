#pragma once

namespace ggl {

class texture;

struct sprite
{
	sprite(const texture *tex, int left, int top, int width, int height);

	const texture *tex;
	int width, height;
	float u0, u1, v0, v1;
};

}
