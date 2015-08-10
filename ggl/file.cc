#include <unistd.h>

#include <ggl/panic.h>
#include <ggl/file.h>

namespace ggl {

uint8_t
file::read_uint8()
{
	uint8_t v;

	if (read(&v, 1) != 1) {
		panic("read failed");
	}

	return v;
}

uint16_t
file::read_uint16()
{
	uint16_t lo = static_cast<uint16_t>(read_uint8());
	uint16_t hi = static_cast<uint16_t>(read_uint8());
	return lo | (hi << 8);
}

uint32_t
file::read_uint32()
{
	uint32_t lo = static_cast<uint32_t>(read_uint16());
	uint32_t hi = static_cast<uint32_t>(read_uint16());
	return lo | (hi << 16);
}

size_t
file::read(void *buf, size_t size)
{
	return ::fread(buf, 1, size, fp);
}

}
