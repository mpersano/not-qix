#pragma once

class game;

namespace ggl {
class sprite_batch;
};

class widget
{
public:
	widget(game& g);
	virtual ~widget() = default;

	virtual bool update() = 0;
	virtual void draw(ggl::sprite_batch& sb) const = 0;

protected:
	game& game_;
};
