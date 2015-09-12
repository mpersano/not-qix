#pragma once

class effect
{
public:
	virtual ~effect() = default;

	virtual bool update() = 0;
	virtual void draw() const = 0;
};
