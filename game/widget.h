#pragma once

class widget
{
public:
	virtual ~widget() = default;

	virtual bool update() = 0;
	virtual void draw() const = 0;
};
