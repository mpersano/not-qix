#pragma once

class effect
{
public:
	virtual ~effect() = default;

	virtual bool update(float dt) = 0;
	virtual void draw() const = 0;
};
