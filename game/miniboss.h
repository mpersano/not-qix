#pragma once

#include "script_interface.h"
#include "foe.h"

namespace ggl {
class mesh;
}

class boss;

class miniboss : public foe
{
public:
	miniboss(game& g, const vec2f& pos);

	void draw() const override;
	bool update() override;

	static const int RADIUS = 26;

private:
	bool intersects_children(const vec2i& from, const vec2i& to) const override;
	bool intersects_children(const vec2i& center, float radius) const override;

	boss *parent_;
	const ggl::mesh *mesh_, *outline_mesh_;
	float ax_, ay_;

	std::unique_ptr<script_thread> script_thread_;
};
