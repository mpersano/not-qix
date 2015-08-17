#pragma once

#include <ggl/noncopyable.h>
#include <ggl/vec2.h>

class game;

class foe : private ggl::noncopyable
{
public:
	foe(game& g);
	virtual ~foe() = default;

	virtual void draw() const = 0;
	virtual bool update() = 0;

	virtual bool is_boss() const = 0;

	virtual bool intersects(const vec2i& from, const vec2i& to) const = 0;

protected:
	game& game_;
};

class phys_foe : public foe
{
public:
	phys_foe(game& g, const vec2f& pos, const vec2f& dir, float speed, float radius);

	bool intersects(const vec2i& from, const vec2i& to) const override;

	void move();

	vec2f get_position() const;

protected:
	vec2f pos_;
	vec2f dir_;
	float speed_;
	float radius_;
};
