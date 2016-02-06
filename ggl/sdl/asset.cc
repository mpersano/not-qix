#include <cstring>

#include <sys/stat.h>
#include <unistd.h>

#include <ggl/panic.h>
#include <ggl/sdl/asset.h>

namespace ggl { namespace sdl {

asset::asset(const std::string& path)
: file_(PHYSFS_openRead(path.c_str()))
{
	if (!file_)
		panic("failed to open `%s': %s", path.c_str(), PHYSFS_getLastError());

	size_ = PHYSFS_fileLength(file_);
}

asset::~asset()
{
	PHYSFS_close(file_);
}

size_t
asset::read(void *buf, size_t size)
{
	return PHYSFS_read(file_, buf, 1, size);
}

off_t
asset::size() const
{
	return size_;
}

} }
