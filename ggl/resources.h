#pragma once

#include <string>

namespace ggl {
class texture;
class font;
}

namespace ggl { namespace res {

void
init();

const texture *
get_texture(const std::string& name);

const font *
get_font(const std::string& name);

} }
