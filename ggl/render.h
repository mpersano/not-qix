#pragma once

#include <ggl/vec2.h>

namespace ggl {

class texture;
class rgba;

struct bbox
{
	vec2f min, max;
};

struct quad
{
	vec2f p0, p1, p2, p3;
};

namespace render {

void
init();

void
set_viewport(const bbox& viewport);

void
begin();

void
translate(const vec2f& p);

void
translate(float x, float y);

void
scale(const vec2f& s);

void
scale(float sx, float sy);

void
scale(float s);

void
rotate(float a);

void
push_matrix();

void
pop_matrix();

void
set_color(const rgba& color);

void
draw(const texture *tex, const bbox& tex_coords, const bbox& dest_coords, float depth);

void
draw(const texture *tex, const bbox& tex_coords, const quad& dest_coords, float depth);

void
draw(const texture *tex0, const texture *tex1, const bbox& tex0_coords, const bbox& tex1_coords, const bbox& dest_coords, float depth);

void
draw(const texture *tex0, const texture *tex1, const bbox& tex0_coords, const bbox& tex1_coords, const quad& dest_coords, float depth);

void
end();

} }
