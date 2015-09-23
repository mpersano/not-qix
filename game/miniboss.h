#pragma once

#include "foe.h"

namespace ggl {
class sprite;
}

class boss;

class miniboss : public foe
{
public:
	miniboss(game& g, boss *b, const vec2f& pos, const vec2f& dir);

	void draw() const override;
	bool update() override;

private:
	boss *boss_;
	const ggl::sprite *sprite_;
};
