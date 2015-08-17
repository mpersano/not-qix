#include <ggl/panic.h>
#include <ggl/android/asset.h>

namespace ggl { namespace android {

asset::asset(AAssetManager *asset_manager, const std::string& path)
: asset_ { AAssetManager_open(asset_manager, path.c_str(), AASSET_MODE_UNKNOWN) }
{
	if (!asset_)
		panic("failed to open `%s'", path.c_str());

	size_ = AAsset_getLength(asset_);
}

asset::~asset()
{
	AAsset_close(asset_);
}

size_t
asset::read(void *buf, size_t size)
{
	return AAsset_read(asset_, buf, size);
}

off_t
asset::size() const
{
	return size_;
}

} }
