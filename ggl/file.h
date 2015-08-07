#pragma once

#include <cstdio>
#include <cstdint>
#include <string>

namespace ggl {

struct file
{
	file(const char *path, const char *mode)
	: fp(fopen(path, mode))
	{ }

	file(const file&) = delete;
	file& operator=(const file&) = delete;

	~file()
	{ fclose(fp); }

	operator bool() const
	{ return fp != nullptr; }

	FILE *fp;
};

} // ggl
