#include <vector>
#include <memory>

#include <ggl/gl_program.h>
#include <ggl/log.h>

#include "programs.h"

namespace ggl { namespace programs {

namespace {

const char *vert_shader_flat =
#ifdef ANDROID
	"#version 300 es\n"
#else
	"#version 430 core\n"
#endif
	"uniform mat4 proj_modelview;\n"
	"\n"
	"layout(location=0) in vec2 position;\n"
	"\n"
	"void main(void)\n"
	"{\n"
	"	gl_Position = proj_modelview*vec4(position, 0, 1);\n"
	"}";

const char *frag_shader_flat =
#ifdef ANDROID
	"#version 300 es\n"
#else
	"#version 430 core\n"
#endif
	"uniform vec4 color;\n"
	"\n"
	"out vec4 out_color;\n"
	"\n"
	"void main(void)\n"
	"{\n"
	"	out_color = color;\n"
	"}";

const char *vert_shader_texture_decal =
#ifdef ANDROID
	"#version 300 es\n"
#else
	"#version 430 core\n"
#endif
	"uniform mat4 proj_modelview;\n"
	"\n"
	"layout(location=0) in vec2 position;\n"
	"layout(location=1) in vec2 texcoord;\n"
	"\n"
	"out vec2 frag_texcoord;\n"
	"\n"
	"void main(void)\n"
	"{\n"
	"	gl_Position = proj_modelview*vec4(position, 0, 1);\n"
	"	frag_texcoord = texcoord;\n"
	"}";

const char *frag_shader_texture_decal =
#ifdef ANDROID
	"#version 300 es\n"
#else
	"#version 430 core\n"
#endif
	"uniform sampler2D tex;\n"
	"\n"
	"in vec2 frag_texcoord;\n"
	"\n"
	"out vec4 out_color;\n"
	"\n"
	"void main(void)\n"
	"{\n"
	"	out_color = texture(tex, frag_texcoord);\n"
	"}";

const char *vert_shader_texture_color =
#ifdef ANDROID
	"#version 300 es\n"
#else
	"#version 430 core\n"
#endif
	"uniform mat4 proj_modelview;\n"
	"\n"
	"layout(location=0) in vec2 position;\n"
	"layout(location=1) in vec2 texcoord;\n"
	"layout(location=2) in vec4 color;\n"
	"\n"
	"out vec2 frag_texcoord;\n"
	"out vec4 frag_color;\n"
	"\n"
	"void main(void)\n"
	"{\n"
	"	gl_Position = proj_modelview*vec4(position, 0, 1);\n"
	"	frag_texcoord = texcoord;\n"
	"	frag_color = color;\n"
	"}";

const char *frag_shader_texture_color =
#ifdef ANDROID
	"#version 300 es\n"
#else
	"#version 430 core\n"
#endif
	"uniform sampler2D tex;\n"
	"\n"
	"in vec2 frag_texcoord;\n"
	"in vec4 frag_color;\n"
	"\n"
	"out vec4 out_color;\n"
	"\n"
	"void main(void)\n"
	"{\n"
	"	out_color = texture(tex, frag_texcoord)*frag_color;\n"
	"}";

const char *vert_shader_multitexture_color =
#ifdef ANDROID
	"#version 300 es\n"
#else
	"#version 430 core\n"
#endif
	"uniform mat4 proj_modelview;\n"
	"\n"
	"layout(location=0) in vec2 position;\n"
	"layout(location=1) in vec2 texcoord0;\n"
	"layout(location=2) in vec2 texcoord1;\n"
	"layout(location=3) in vec4 color;\n"
	"\n"
	"out vec2 frag_texcoord0;\n"
	"out vec2 frag_texcoord1;\n"
	"out vec4 frag_color;\n"
	"\n"
	"void main(void)\n"
	"{\n"
	"	gl_Position = proj_modelview*vec4(position, 0, 1);\n"
	"	frag_texcoord0 = texcoord0;\n"
	"	frag_texcoord1 = texcoord1;\n"
	"	frag_color = color;\n"
	"}";

const char *frag_shader_multitexture_color =
#ifdef ANDROID
	"#version 300 es\n"
#else
	"#version 430 core\n"
#endif
	"uniform sampler2D tex0;\n"
	"uniform sampler2D tex1;\n"
	"\n"
	"in vec2 frag_texcoord0;\n"
	"in vec2 frag_texcoord1;\n"
	"in vec4 frag_color;\n"
	"\n"
	"out vec4 out_color;\n"
	"\n"
	"void main(void)\n"
	"{\n"
	"	vec4 c0 = texture(tex0, frag_texcoord0);\n"
	"	vec4 c1 = texture(tex1, frag_texcoord1);\n"
	"	out_color = vec4(c0.rgb + c1.rgb, c0.a)*frag_color;\n"
	"}";

std::vector<std::unique_ptr<gl_program>> g_programs;

} // (anonymous namespace)

void
init()
{
	struct { const char *vert_source, *frag_source; } programs[program_type::NUM_PROGRAMS]
	{
		{ vert_shader_flat, frag_shader_flat },
		{ vert_shader_texture_decal, frag_shader_texture_decal },
		{ vert_shader_texture_color, frag_shader_texture_color },
		{ vert_shader_multitexture_color, frag_shader_multitexture_color },
	};

	for (auto& p : programs) {
		log_info("compiling...");

		gl_shader vert_single { GL_VERTEX_SHADER };
		vert_single.set_source(p.vert_source);
		vert_single.compile();

		gl_shader frag_single { GL_FRAGMENT_SHADER };
		frag_single.set_source(p.frag_source);
		frag_single.compile();

		std::unique_ptr<gl_program> prog { new gl_program };

		prog->attach(vert_single);
		prog->attach(frag_single);
		prog->link();

		g_programs.push_back(std::move(prog));
	}
}

const gl_program *
get_program(program_type p)
{
	return g_programs[p].get();
}

} }
