#pragma once

#include <functional>

#include "foe.h"

namespace ggl {
class sprite;
}

class miniboss : public phys_foe
{
public:
	miniboss(game& g, const vec2f& pos, const vec2f& dir, const std::function<void(void)>& on_death);

	void draw() const override;
	bool update() override;

	bool is_boss() const
	{ return false; }

private:
	const ggl::sprite *sprite_;
	std::function<void(void)> on_death_;
};
