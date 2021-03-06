#pragma once

#include <ggl/gl.h>
#include <ggl/vec2.h>
#include <ggl/vec3.h>
#include <ggl/mat3.h>

namespace ggl {

class texture;
class rgba;
class mesh;

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
shutdown();

void
set_viewport(const bbox& viewport);

bbox
get_viewport();

mat3
get_matrix();

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

// untextured quads

void
draw(const bbox& dest_coords, float depth);

void
draw(const quad& dest_coords, float depth);

// textured quads

void
draw(const texture *tex, const bbox& tex_coords, const bbox& dest_coords, float depth);

void
draw(const texture *tex, const bbox& tex_coords, const quad& dest_coords, float depth);

// 2-texture quads

void
draw(const texture *tex0, const texture *tex1, const bbox& tex0_coords, const bbox& tex1_coords, const bbox& dest_coords, float depth);

void
draw(const texture *tex0, const texture *tex1, const bbox& tex0_coords, const bbox& tex1_coords, const quad& dest_coords, float depth);

// mesh

void
draw(const mesh *m, const mat4& mat, float depth);

void
end();

const GLfloat *
get_proj_modelview();

} }
