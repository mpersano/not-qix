#pragma once

class widget
{
public:
	virtual ~widget() = default;

	virtual void hide() = 0;
	virtual void show() = 0;

	virtual bool update() = 0;
	virtual void draw() const = 0;
};
