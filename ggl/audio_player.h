#pragma once

#include <string>

namespace ggl {

class audio_player
{
public:
	audio_player() { }
	virtual ~audio_player() { }

	virtual void open(const std::string& path) = 0;
	virtual void close() = 0;

	virtual void start() = 0;
	virtual void stop() = 0;

	virtual void update() = 0;

	virtual void set_gain(float g) = 0;
	virtual void fade_out(int ttl) = 0;
};

}
