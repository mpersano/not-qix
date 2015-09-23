#pragma once

#include <ggl/noncopyable.h>

class effect : private ggl::noncopyable
{
public:
	virtual ~effect() = default;

	virtual bool update() = 0;
	virtual void draw() const = 0;

	virtual bool is_position_absolute() const = 0;
};
