#pragma once

#include <ggl/noncopyable.h>
#include <ggl/vec2.h>

class game;

class foe : private ggl::noncopyable
{
public:
	foe(game& g);

	void draw() const;
	bool update(float dt);

private:
	game& game_;
	vec2f position_;
	vec2f dir_;
	float speed_;
	float radius_;
};
