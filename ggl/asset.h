#pragma once

#include <cstdint>
#include <cstddef>

#include <ggl/noncopyable.h>

namespace ggl {

class asset : private noncopyable
{
public:
	virtual ~asset() = default;

	virtual size_t read(void *buf, size_t size) = 0;

	uint8_t read_uint8();
	uint16_t read_uint16();
	uint32_t read_uint32();
};

}
