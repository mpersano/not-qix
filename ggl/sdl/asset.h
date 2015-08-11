#include <cstdint>
#include <cstdio>

#include <string>

#include <ggl/asset.h>

namespace ggl { namespace sdl {

class asset : public ggl::asset
{
public:
	asset(const std::string& path);
	~asset();

	size_t read(void *buf, size_t size) override;

private:
	FILE *stream_;
};

} }
