#include <cmath>
#include <ggl/asset.h>

namespace {

float
ieee_to_float(uint32_t bits)
{
        if (bits == 0)
                return 0;
        float mantissa = .5 + static_cast<float>(bits & ((1 << 23) - 1))/0x1000000;
        int expon = ((bits >> 23) & 0xff) - 126;
        float val = ldexpf(mantissa, expon);
        return (bits & 0x80000000) ? -val : val;
}

}

namespace ggl {

std::vector<char>
asset::read_all()
{
	auto s = size();

	std::vector<char> data(s);
	read(&data[0], s);

	return data;
}

uint8_t
asset::read_uint8()
{
	uint8_t v;
	read(&v, 1);
	return v;
}

uint16_t
asset::read_uint16()
{
	uint8_t lo = read_uint8();
	uint8_t hi = read_uint8();
	return lo | (hi << 8);
}

uint32_t
asset::read_uint32()
{
	uint16_t lo = read_uint16();
	uint16_t hi = read_uint16();
	return lo | (hi << 16);
}

float
asset::read_float()
{
	uint32_t v = read_uint32();
	return ieee_to_float(v);
}

}
