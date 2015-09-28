#pragma once

#include "foe.h"

namespace ggl {
class sprite;
}

class boss;

class miniboss : public foe
{
public:
	miniboss(game& g, const vec2f& pos, boss *parent);

	void draw() const override;
	bool update() override;

private:
	bool intersects_children(const vec2i& from, const vec2i& to) const override;
	bool intersects_children(const vec2i& center, float radius) const override;

	boss *parent_;
	const ggl::sprite *sprite_;
};
