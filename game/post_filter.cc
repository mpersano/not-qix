#include <ggl/resources.h>
#include <ggl/render_target.h>
#include <ggl/framebuffer.h>
#include <ggl/vertex_array.h>
#include <ggl/program.h>

#include "post_filter.h"

static const ggl::vertex_array_texcoord<GLshort, 2, GLshort, 2> fullscreen_va
	{ { -1, -1, 0, 0 }, { -1, 1, 0, 1 }, { 1, -1, 1, 0 }, { 1, 1, 1, 1 } };

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

dynamic_post_filter::dynamic_post_filter(const char *program)
: post_filter { program }
{ }

ripple_filter::ripple_filter(float width, const vec2f& center, float speed, int ttl)
: dynamic_post_filter { "ripple-filter" }
, width_ { width }
, center_ { center }
, radius_ { 0 }
, speed_ { speed }
, tics_ { 0 }
, ttl_ { ttl }
{
	program_->use();
	program_->set_uniform_i("source_buffer", 0);
}

bool
ripple_filter::update()
{
	radius_ += speed_;
	return ++tics_ < ttl_;
}

void
ripple_filter::draw(const ggl::framebuffer& source, const ggl::render_target& dest) const
{
	program_->use();
	program_->set_uniform_f("resolution", source.get_width(), source.get_height());
	program_->set_uniform_f("width", width_);
	program_->set_uniform_f("center", center_.x, center_.y);
	program_->set_uniform_f("radius", radius_);
	program_->set_uniform_f("scale", .02f*(1.f - static_cast<float>(tics_)/ttl_));

	dest.bind();
	source.bind_texture();
	fullscreen_va.draw(GL_TRIANGLE_STRIP);
}
