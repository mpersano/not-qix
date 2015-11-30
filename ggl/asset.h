#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include <sys/types.h> // off_t

#include <ggl/noncopyable.h>

namespace ggl {

class asset : private noncopyable
{
public:
	virtual ~asset() = default;

	virtual off_t size() const = 0;
	virtual size_t read(void *buf, size_t size) = 0;

	std::vector<char> read_all();

	uint8_t read_uint8();
	uint16_t read_uint16();
	uint32_t read_uint32();
	float read_float();
};

}
