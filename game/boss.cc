#include <cmath>
#include <cstdlib>
#include <cassert>

#include <algorithm>

#include <ggl/vec2_util.h>
#include <ggl/texture.h>
#include <ggl/sprite.h>
#include <ggl/mesh.h>
#include <ggl/resources.h>
#include <ggl/render.h>
#include <ggl/util.h>

#include "tween.h"
#include "game.h"
#include "fake3d.h"
#include "boss.h"

namespace {

class bullet : public entity
{
public:
	bullet(game& g, const vec2f& pos, const vec2f& dir);

	void draw() const override;
	bool update() override;

	bool intersects(const vec2i& from, const vec2i& to) const override;
	bool intersects(const vec2i& center, float radius) const override;

private:
	static const int LENGTH = 20;

	vec2f pos_;
	vec2f dir_;
	const ggl::sprite *sprite_;
};

bullet::bullet(game& g, const vec2f& pos, const vec2f& dir)
: entity { g }
, pos_ { pos }
, dir_ { dir }
, sprite_ { ggl::res::get_sprite("bullet.png") }
{ }

void
bullet::draw() const
{
	const int w = sprite_->width;
	const int h = sprite_->height;

	vec2f up = vec2f { -dir_.y, dir_.x }*.5f*h;
	vec2f right = dir_*static_cast<float>(w);

	const vec2f p0 = pos_ + up;
	const vec2f p1 = pos_ - up;
	const vec2f p2 = pos_ - up + right;
	const vec2f p3 = pos_ + up + right;

	const float u0 = sprite_->u0;
	const float u1 = sprite_->u1;

	const float v0 = sprite_->v0;
	const float v1 = sprite_->v1;

	ggl::render::draw(sprite_->tex, { { u0, v1 }, { u1, v0 } }, ggl::quad { p0, p1, p2, p3 }, 0);
}

bool
bullet::update()
{
	static const float SPEED = 4;

	pos_ += SPEED*dir_;

	// outside grid?

	int grid_width = game_.grid_cols*CELL_SIZE;
	int grid_height = game_.grid_rows*CELL_SIZE;

	vec2f end = pos_ + dir_*static_cast<float>(LENGTH);

	if (std::min(pos_.x, end.x) > grid_width ||
		std::max(pos_.x, end.x) < 0 ||
		std::min(pos_.y, end.y) > grid_height ||
		std::max(pos_.y, end.y) < 0) {
		return false;
	}

	return true;
}

bool
bullet::intersects(const vec2i& from, const vec2i& to) const
{
	vec2f end = pos_ + dir_*static_cast<float>(LENGTH);

	// i'm sure there's a cleaner way to do this...

	float d = (end.y - pos_.y)*(to.x - from.x) - (end.x - pos_.x)*(to.y - from.y);
	if (d == 0)
		return false;

	float ua = ((end.x - pos_.x)*(from.y - pos_.y) - (end.y - pos_.y)*(from.x - pos_.x))/d;
	float ub = ((to.x - from.x)*(from.y - pos_.y) - (to.y - from.y)*(from.x - pos_.x))/d;

	return ua >= 0 && ua <= 1 && ub >= 0 && ub <= 1;
}

bool
bullet::intersects(const vec2i& center, float radius) const
{
	vec2f end = pos_ + dir_*static_cast<float>(LENGTH);

	return distance(seg_closest_point(pos_, end, center), vec2f(center)) < radius;
}

// pod formations

const float POD_DISTANCE = 36;
const float LASER_DISTANCE = 20;

} // (anonymous namespace)

//
//  b o s s
//

boss::boss(game& g, const vec2f& pos)
: foe { g, pos, RADIUS }
, pod_angle_ { 0 }
, core_sprite_ { ggl::res::get_sprite("boss-core.png") }
, core_mesh_ { ggl::res::get_mesh("meshes/boss.msh") }
, core_outline_mesh_ { ggl::res::get_mesh("meshes/boss-outline.msh") }
, danger_up_sprite_ { ggl::res::get_sprite("danger-up.png") }
, danger_down_sprite_ { ggl::res::get_sprite("danger-down.png") }
, script_thread_ { create_script_thread("scripts/boss.lua") }
{
	for (int i = 0; i < NUM_PODS; i++)
		pods_.push_back(std::unique_ptr<pod>(new pod { game_ }));

	script_thread_->call("init", this);
}

bool
boss::intersects_children(const vec2i& from, const vec2i& to) const
{
	return std::find_if(
		std::begin(pods_),
		std::end(pods_),
		[&](const std::unique_ptr<pod>& p) { return p->intersects(pos_, pod_angle_, from, to); }) != std::end(pods_);
}

bool
boss::intersects_children(const vec2i& center, float radius) const
{
	return false;
}

bool
boss::update()
{
	script_thread_->call("update", this);

	for (auto& p : pods_)
		p->update();

	return true;
}

void
boss::set_pod_position(int pod, float da, float r)
{
	auto& p = pods_[pod];
	p->ang_offset = da;
	p->rotation = r;
}

void
boss::set_pod_angle(float a)
{
	pod_angle_ = a;
}

void
boss::rotate_pods(float da)
{
	pod_angle_ += da;
}

void
boss::rotate_pods_to_player()
{
	const vec2f d = vec2f(game_.get_player_world_position()) - pos_;
	const vec2f n = normalized(vec2f { -d.y, d.x });
	const vec2f u { cosf(pod_angle_), sinf(pod_angle_) };

	pod_angle_ -= .05f*dot(n, u);
}

void
boss::fire_bullet(int pod)
{
	pods_[pod]->fire_bullet(pos_, pod_angle_);
}

void
boss::fire_laser(int pod, float power)
{
	pods_[pod]->fire_laser(power);
}

void
boss::draw() const
{
	draw_core();
	draw_pods();
}

void
boss::draw_core() const
{
	auto screen_pos = pos_ + game_.offset;

	if (screen_pos.y + radius_ < 0) {
		auto p = vec2f { screen_pos.x, 0 } - game_.offset;
		danger_down_sprite_->draw(0, p, ggl::vert_align::BOTTOM, ggl::horiz_align::CENTER);
	} else if (screen_pos.y - radius_ > game_.viewport_height) {
		auto p = vec2f { screen_pos.x, game_.viewport_height } - game_.offset;
		danger_up_sprite_->draw(0, p, ggl::vert_align::TOP, ggl::horiz_align::CENTER);
	} else {
		mat4 m =
			mat4::rotation_around_x(.5*M_PI)*
			mat4::rotation_around_y(pod_angle_)*
			mat4::rotation_around_z(.3*pod_angle_)*
			mat4::scale(2.5, 2.5, 2.5);

		draw_mesh_outline(core_outline_mesh_, pos_, m, .5);
		draw_mesh(core_mesh_, pos_, m);

		// core_sprite_->draw(0, pos_);
	}
}

void
boss::draw_pods() const
{
	ggl::render::push_matrix();
	ggl::render::translate(pos_);
	ggl::render::rotate(pod_angle_ - .5f*M_PI);

	for (auto& p : pods_)
		p->draw();

	ggl::render::pop_matrix();
}

namespace {

const int MUZZLE_FLASH_TICS = 6;

} // (anonymous namespace)

boss::pod::pod(game& g)
: ang_offset { 0 }
, rotation { 0 }
, game_ { g }
, sprite_ { ggl::res::get_sprite("boss-spike.png") }
, muzzle_flash_sprite_ { ggl::res::get_sprite("muzzle-flash.png") }
, laser_flash_sprite_ { ggl::res::get_sprite("laser-flash.png") }
, laser_segment_texture_ { ggl::res::get_texture("images/laser-segment.png") }
, fire_tics_ { 0 }
, laser_power_ { 0 }
{ }

namespace {
bool touched;
vec2f touch_pos;
}

void
boss::pod::draw() const
{
	ggl::render::push_matrix();

	ggl::render::rotate(ang_offset);
	ggl::render::translate(0, POD_DISTANCE);
	ggl::render::rotate(rotation);

	sprite_->draw(0, { 0, 0 }, ggl::vert_align::BOTTOM, ggl::horiz_align::CENTER);

	if (fire_tics_) {
		const float t = static_cast<float>(fire_tics_)/MUZZLE_FLASH_TICS;

		ggl::render::push_matrix();
		ggl::render::translate(0, LASER_DISTANCE);
		ggl::render::scale(t*1.1f);
		muzzle_flash_sprite_->draw(1);
		ggl::render::pop_matrix();
	}

	if (laser_power_) {
		float radius = get_laser_radius();
		ggl::render::draw(laser_segment_texture_, { { 0, 0 }, { 1, 1 } }, ggl::bbox { { -radius, LASER_DISTANCE }, { radius, 1000 } }, 0);

		float r = laser_power_*1.1f*(1.f + .15f*sinf(.1f*game_.tics));

		ggl::render::push_matrix();
		ggl::render::translate(0, LASER_DISTANCE);
		ggl::render::scale(r);
		laser_flash_sprite_->draw(1);
		ggl::render::pop_matrix();
	}

	ggl::render::pop_matrix();
}

void
boss::pod::update()
{
	if (fire_tics_ > 0)
		--fire_tics_;
}

void
boss::pod::fire_bullet(const vec2f& center, float angle)
{
	const float a = angle + ang_offset;
	const vec2f d = { cosf(a), sinf(a) };
	game_.add_entity(std::unique_ptr<entity>(new bullet { game_, center + d*(POD_DISTANCE + LASER_DISTANCE), d }));

	fire_tics_ = MUZZLE_FLASH_TICS;
}

void
boss::pod::fire_laser(float power)
{
	laser_power_ = power;
}

bool
boss::pod::intersects(const vec2f& center, float angle, const vec2i& from, const vec2i& to) const
{
	if (laser_power_ == 0.f)
		return false;

	const float a = angle - .5f*M_PI + ang_offset;

	vec2f v0 = (vec2f(from) - center).rotate(-a);
	vec2f v1 = (vec2f(to) - center).rotate(-a);

	const float min_y = POD_DISTANCE + LASER_DISTANCE;

	if (v0.y <= min_y) {
		if (v1.y <= min_y)
			return false;

		v0.x = (min_y - v0.y)*(v1.x - v0.x)/(v1.y - v0.y) + v0.x;
		v0.y = min_y;
	} else if (v1.y <= min_y) {
		assert(v0.y > min_y);

		v1.x = (min_y - v0.y)*(v1.x - v0.x)/(v1.y - v0.y) + v0.x;
		v1.y = min_y;
	}

	float radius = get_laser_radius();

	return !(std::max(v0.x, v1.x) < -radius || std::min(v0.x, v1.x) > radius);
}

float
boss::pod::get_laser_radius() const
{
	return 20.f*laser_power_;
}
