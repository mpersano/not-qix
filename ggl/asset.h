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
	virtual uint8_t read_uint8() = 0;
	virtual uint16_t read_uint16() = 0;
	virtual uint32_t read_uint32() = 0;
};

}
