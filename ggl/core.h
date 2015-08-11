#pragma once

#include <memory>

#include <ggl/app.h>

namespace ggl {

class asset;

enum {
	DPAD_UP = 1,
	DPAD_DOWN = 2,
	DPAD_LEFT = 4,
	DPAD_RIGHT = 8,
	DPAD_BUTTON1 = 16,
	DPAD_BUTTON2 = 32,
};

class core
{
public:
	core(app& a) : app_(a) { }
	virtual ~core() = default;

	virtual void run() = 0;

	virtual int get_viewport_width() const = 0;
	virtual int get_viewport_height() const = 0;

	virtual unsigned get_dpad_state() const = 0;

	virtual std::unique_ptr<asset> get_asset(const std::string& path) const = 0;

	virtual float now() const = 0;

protected:
	app& app_;
};

extern core *g_core;

}
