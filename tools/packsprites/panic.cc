#include <cstdio>
#include <cstdlib>
#include <cstdarg>

#include "panic.h"

void
panic(const char *fmt, ...)
{
	char buf[512];

	va_list ap;

	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);

	fprintf(stderr, "FATAL: %s\n", buf);

	exit(1);
}
