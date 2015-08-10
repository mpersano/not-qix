#include <cstdio>
#include <cstdlib>

#include <stdarg.h>

#include <ggl/log.h>
#include <ggl/panic.h>

void
panic(const char *fmt, ...)
{
	char buf[2048];

	va_list ap;

	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);

	log_error("%s", buf);

	exit(1);
}
