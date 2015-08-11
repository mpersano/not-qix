#include <cstring>

#include <ggl/panic.h>
#include <ggl/sdl/asset.h>

namespace ggl { namespace sdl {

asset::asset(const std::string& path)
: stream_(fopen(path.c_str(), "rb"))
{
	if (!stream_)
		panic("failed to open `%s': %s", path.c_str(), strerror(errno));
}

asset::~asset()
{
	fclose(stream_);
}

size_t
asset::read(void *buf, size_t size)
{
	return ::fread(buf, 1, size, stream_);
}

} }
