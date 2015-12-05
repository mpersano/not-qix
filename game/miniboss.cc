#include <ggl/gl_check.h>
#include <ggl/mesh.h>
#include <ggl/resources.h>

#include "game.h"
#include "boss.h"
#include "fake3d.h"
#include "explosion.h"
#include "miniboss.h"

miniboss::miniboss(game& g, const vec2f& pos)
: foe { g, pos, RADIUS }
, script_thread_ { create_script_thread("scripts/miniboss.lua") }
, mesh_ { ggl::res::get_mesh("meshes/miniboss.msh") }
, outline_mesh_ { ggl::res::get_mesh("meshes/miniboss-outline.msh") }
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

	draw_mesh_outline(outline_mesh_, pos_, m, .25);
	draw_mesh(mesh_, pos_, m);
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
				game_.add_effect(std::unique_ptr<effect>(new explosion(pos_, 2)));
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
