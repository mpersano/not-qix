#include <ggl/gl_check.h>
#include <ggl/mesh.h>
#include <ggl/resources.h>

#include "game.h"
#include "debuggfx.h"
#include "post_filter.h"
#include "explosion.h"
#include "miniboss.h"

miniboss::miniboss(game& g, const vec2f& pos)
: foe { g, pos, RADIUS }
, script_thread_ { create_script_thread("scripts/miniboss.lua") }
, mesh_ { ggl::res::get_mesh("meshes/miniboss.msh") }
, ax_ { 0 }
, ay_ { 0 }
{
	script_thread_->call("init", this);
}
void
miniboss::draw() const
{
	mat4 m =
		mat4::rotation_around_x(ax_)*
		mat4::rotation_around_y(ay_)*
		mat4::scale(2.5, 2.5, 2.5);

	ggl::render::push_matrix();
	ggl::render::translate(pos_);
	ggl::render::draw(mesh_, m, 0.f);
	ggl::render::pop_matrix();

	// draw_circle(pos_, RADIUS, 4.f);
}

bool
miniboss::update()
{
	// check if killed

	auto p0 = vec2i((pos_ - vec2f { .5f, .5f }*radius_)/CELL_SIZE);
	auto p1 = vec2i((pos_ + vec2f { .5f, .5f }*radius_)/CELL_SIZE);

	for (int r = p0.y; r <= p1.y; r++) {
		for (int c = p0.x; c <= p1.x; c++) {
			if (game_.grid[r*game_.grid_cols + c]) {
				printf("killed!\n");
				game_.add_effect(std::unique_ptr<effect>(new explosion(pos_, 1)));
				game_.add_post_filter(std::unique_ptr<dynamic_post_filter>(new ripple_filter(30., pos_ + game_.offset, 3.f, 100.f)));
				game_.start_screenshake(30, 20.f);
				game_.start_screenflash(10);
				return false;
			}
		}
	}

	ax_ += .01;
	ay_ += .02;

	script_thread_->call("update", this);

	return true;
}

bool
miniboss::intersects_children(const vec2i& from, const vec2i& to) const
{
	return false;
}

bool
miniboss::intersects_children(const vec2i& center, float radius) const
{
	return false;
}
