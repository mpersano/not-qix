#include <ggl/resources.h>
#include <ggl/render_target.h>
#include <ggl/framebuffer.h>
#include <ggl/vertex_array.h>
#include <ggl/program.h>

#include "post_filter.h"

static const ggl::vertex_array_texcoord<GLshort, 2, GLshort, 2> fullscreen_va
	{ { -1, -1, 0, 0 }, { -1,  1, 0, 1 }, {  1, -1, 1, 0 }, {  1,  1, 1, 1 } };

post_filter::post_filter(const char *program)
: program_ { ggl::res::get_program(program) }
{ }

passthru_filter::passthru_filter()
: post_filter { "passthru-filter" }
{
	program_->use();
	program_->set_uniform_i("source_buffer", 0);
}

void
passthru_filter::draw(const ggl::framebuffer& source, const ggl::render_target& dest) const
{
	program_->use();

	dest.bind();
	source.bind_texture();
	fullscreen_va.draw(GL_TRIANGLE_STRIP);
}

ripple_filter::ripple_filter()
: post_filter { "ripple-filter" }
{
	program_->use();
	program_->set_uniform_i("source_buffer", 0);
}

void
ripple_filter::set_params(float width, const vec2f& center, float radius) const
{
	program_->use();
	program_->set_uniform_f("width", width);
	program_->set_uniform_f("center", center.x, center.y);
	program_->set_uniform_f("radius", radius);
}

void
ripple_filter::draw(const ggl::framebuffer& source, const ggl::render_target& dest) const
{
	program_->use();
	program_->set_uniform_f("resolution", source.get_width(), source.get_height());

	dest.bind();
	source.bind_texture();
	fullscreen_va.draw(GL_TRIANGLE_STRIP);
}
