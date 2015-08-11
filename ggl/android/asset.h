#include <string>

#include <ggl/asset.h>

#include <android/asset_manager.h>

namespace ggl { namespace android {

class asset : public ggl::asset
{
public:
	asset(AAssetManager *asset_manager, const std::string& path);
	~asset();

	size_t read(void *buf, size_t size) override;

private:
	AAsset *asset_;
};

} }
