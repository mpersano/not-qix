#pragma once

class game;

class widget
{
public:
	widget(game& g);
	virtual ~widget() = default;

	virtual bool update() = 0;
	virtual void draw() const = 0;

protected:
	game& game_;
};
