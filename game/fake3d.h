#pragma once

#include <ggl/vec2.h>
#include <ggl/vec3.h>

namespace ggl {
class mesh;
}

void
draw_mesh(const ggl::mesh *mesh, const vec2f& pos, const mat4& transform);
