#pragma once

#include "phys_foe.h"

namespace ggl {
class sprite;
}

class boss;

class miniboss : public phys_foe
{
public:
	miniboss(game& g, boss *b, const vec2f& pos, const vec2f& dir);

	void draw() const override;
	bool update() override;

	bool is_boss() const
	{ return false; }

private:
	boss *boss_;
	const ggl::sprite *sprite_;
};
