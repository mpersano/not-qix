#pragma once

#include <string>

namespace ggl {
class texture;
class font;
class sprite;
}

namespace ggl { namespace res {

void
init();

const texture *
get_texture(const std::string& name);

const font *
get_font(const std::string& name);

const sprite *
get_sprite(const std::string& name);

void
load_sprite_sheet(const std::string& name);

} }
