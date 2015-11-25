#pragma once

namespace ggl {

class gl_program;

namespace programs {

enum program_type {
	FLAT,
	TEXTURE_DECAL,
	TEXTURE_MODULATE,
	MULTITEXTURE_MODULATE,
	NUM_PROGRAMS,
};

void
init();

const gl_program *
get_program(program_type p);

} }
