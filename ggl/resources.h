#pragma once

#include <string>
#include <memory>

namespace ggl {
class texture;
class font;
class sprite;
class action;
class program;
class mesh;
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

const program *
get_program(const std::string& name);

const mesh *
get_mesh(const std::string& name);

std::unique_ptr<action>
get_action(const std::string& name);

void
load_sprite_sheet(const std::string& path);

void
load_programs(const std::string& path);

void
unload_gl_resources();

void
load_gl_resources();

} }
