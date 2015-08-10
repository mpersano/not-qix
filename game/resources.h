#pragma once

#include <string>

#include <ggl/texture.h>

namespace gp { namespace res {

void
init();

const ggl::texture *
get_texture(const std::string& name);

} }
