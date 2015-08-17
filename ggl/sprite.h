#pragma once

namespace ggl {

class texture;

class sprite
{
public:
	sprite(const texture *tex, int left, int top, int width, int height);

	enum class vert_align { TOP, CENTER, BOTTOM };
	enum class horiz_align { LEFT, CENTER, RIGHT };

	void draw() const;
	void draw(horiz_align ha, vert_align va) const;
	void draw(float x, float y) const;
	void draw(float x, float y, horiz_align ha, vert_align va) const;
	void draw(float x0, float y0, float x1, float y1) const;

	const texture *tex;
	int width, height;
	float u0, u1, v0, v1;
};

}
