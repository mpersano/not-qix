#pragma once

#include <cstdio>
#include <stdint.h>

class file_writer
{
public:
	file_writer(const char *path);
	virtual ~file_writer();

	void write_uint8(uint8_t value);
	void write_uint16(uint16_t value);
	void write_uint32(uint32_t value);
	void write_string(const char *str);

private:
	FILE *out_;
};
