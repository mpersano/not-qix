#include <cstring>
#include <cerrno>

#include "panic.h"
#include "file_writer.h"

file_writer::file_writer(const char *path)
: out_(fopen(path, "wb"))
{
	if (!out_)
		panic("failed to open %s: %s\n", path, strerror(errno));
}

file_writer::~file_writer()
{
	fclose(out_);
}

void
file_writer::write_uint8(uint8_t value)
{
	fputc(value, out_);
}

void
file_writer::write_uint16(uint16_t value)
{
	write_uint8(value & 0xff);
	write_uint8(value >> 8);
}

void
file_writer::write_uint32(uint32_t value)
{
	write_uint16(value & 0xff);
	write_uint16(value >> 16);
}

void
file_writer::write_string(const char *str)
{
	const size_t len = strlen(str);

	write_uint8(len);
	fwrite(str, len, 1, out_);
}
