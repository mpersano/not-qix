#include <ggl/panic.h>
#include <ggl/asset.h>

namespace ggl {

uint8_t
asset::read_uint8()
{
	uint8_t v;

	if (read(&v, 1) != 1) {
		panic("read failed");
	}

	return v;
}

uint16_t
asset::read_uint16()
{
	uint16_t lo = static_cast<uint16_t>(read_uint8());
	uint16_t hi = static_cast<uint16_t>(read_uint8());
	return lo | (hi << 8);
}

uint32_t
asset::read_uint32()
{
	uint32_t lo = static_cast<uint32_t>(read_uint16());
	uint32_t hi = static_cast<uint32_t>(read_uint16());
	return lo | (hi << 16);
}

std::string
asset::read_string()
{
	uint8_t len = read_uint8();

	std::string str;
	for (int i = 0; i < len; i++)
		str.push_back(read_uint8());

	return str;
}

}
