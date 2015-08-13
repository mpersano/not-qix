#include <cstring>

#include <sys/stat.h>
#include <unistd.h>

#include <ggl/panic.h>
#include <ggl/sdl/asset.h>

namespace ggl { namespace sdl {

asset::asset(const std::string& path)
: stream_(fopen(path.c_str(), "rb"))
{
	if (!stream_)
		panic("failed to open `%s': %s", path.c_str(), strerror(errno));

	struct stat st;
	if (fstat(fileno(stream_), &st) != 0)
		panic("fstat failed: %s", strerror(errno));

	size_ = st.st_size;
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

off_t
asset::size() const
{
	return size_;
}

} }
