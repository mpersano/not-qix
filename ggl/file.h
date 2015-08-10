#pragma once

#include <cstdio>
#include <cstdint>
#include <string>

#include "noncopyable.h"

namespace ggl {

struct file : private noncopyable
{
	file(const std::string& path)
	: fp(fopen(path.c_str(), "rb"))
	{ }

	file(const file&) = delete;
	file& operator=(const file&) = delete;

	~file()
	{ fclose(fp); }

	operator bool() const
	{ return fp != nullptr; }

	size_t read(void *buf, size_t size);
	uint8_t read_uint8();
	uint16_t read_uint16();
	uint32_t read_uint32();

	FILE *fp;
};

} // ggl
