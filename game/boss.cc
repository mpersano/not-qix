#include <cmath>
#include <cstdlib>

#include <ggl/vec2_util.h>
#include <ggl/texture.h>
#include <ggl/sprite.h>
#include <ggl/resources.h>

#include "tween.h"
#include "game.h"
#include "miniboss.h"
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
	const vec2f p1 = pos_ + up + right;
	const vec2f p2 = pos_ - up;
	const vec2f p3 = pos_ - up + right;

	const float u0 = sprite_->u0;
	const float u1 = sprite_->u1;

	const float v0 = sprite_->v0;
	const float v1 = sprite_->v1;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);

	glColor4f(1, 1, 1, 1);

	sprite_->tex->bind();

	(ggl::vertex_array_texcoord<GLfloat, 2, GLfloat, 2>
		{ { p0.x, p0.y, u0, v1 },
		  { p1.x, p1.y, u1, v1 },
		  { p2.x, p2.y, u0, v0 },
		  { p3.x, p3.y, u1, v0 } }).draw(GL_TRIANGLE_STRIP);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
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

} // (anonymous namespace)

//
//  b o s s
//

boss::boss(game& g, const vec2f& pos)
: foe { g, pos, normalized(vec2f { 1.5f, .5f }), 0, RADIUS }
, pod_angle_ { 0 }
, pods_ { { 0, 0 }, { 0, 0 }, { 0, 0 } }
, miniboss_spawned_ { 0 }
, core_sprite_ { ggl::res::get_sprite("boss-core.png") }
, pod_sprite_ { ggl::res::get_sprite("boss-spike.png") }
, script_thread_ { create_script_thread("scripts/boss.lua") }
{
	script_thread_->call("init", this);
}

bool
boss::update()
{
	script_thread_->call("update", this);
	return true;
}

void
boss::set_pod_position(int pod, float da, float r)
{
	pods_[pod].ang_offset = da;
	pods_[pod].rotation = r;
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
	const float a = pod_angle_ + pods_[pod].ang_offset;
	const vec2f p = pos_ + vec2f { cosf(a), sinf(a) }*POD_DISTANCE;
	vec2f d = normalized(p - pos_);
	game_.add_entity(std::unique_ptr<entity>(new bullet { game_, p, d }));
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
	core_sprite_->draw(pos_.x, pos_.y, ggl::sprite::horiz_align::CENTER, ggl::sprite::vert_align::CENTER);
}

void
boss::draw_pods() const
{
	glPushMatrix();

	glTranslatef(pos_.x, pos_.y, 0.f);
	glRotatef(pod_angle_*180.f/M_PI - 90.f, 0, 0, 1);

	for (auto& p : pods_)
		draw_pod(p.ang_offset, p.rotation);

	glPopMatrix();
}

void
boss::draw_pod(float a, float r) const
{
	glPushMatrix();

	glRotatef(a*180.f/M_PI, 0, 0, 1);
	glTranslatef(0, POD_DISTANCE, 0);
	glRotatef(r*180.f/M_PI, 0, 0, 1);

	pod_sprite_->draw(ggl::sprite::horiz_align::CENTER, ggl::sprite::vert_align::BOTTOM);

	glPopMatrix();
}

void
boss::on_miniboss_killed()
{
	--miniboss_spawned_;
}
