#include <cstdio>

#include <string>

#include <ggl/asset.h>

namespace ggl { namespace sdl {

class asset : public ggl::asset
{
public:
	asset(const std::string& path);
	~asset();

	off_t size() const override;
	size_t read(void *buf, size_t size) override;

private:
	FILE *stream_;
	off_t size_;
};

} }
