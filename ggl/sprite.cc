#include <ggl/texture.h>
#include <ggl/sprite.h>

namespace ggl {

sprite::sprite(const ggl::texture *tex, int left, int top, int width, int height)
: tex { tex }
, width { width }
, height { height }
{
	const int tex_width = tex->width;
	const int tex_height = tex->height;

	const float du = static_cast<float>(width)/tex_width;
	const float dv = static_cast<float>(height)/tex_height;

	u0 = static_cast<float>(left)/tex_width;
	u1 = u0 + du;

	v0 = static_cast<float>(tex_height - top)/tex_height;
	v1 = v0 - dv;
}

}
