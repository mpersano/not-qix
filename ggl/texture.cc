#include <cstdio>
#include <algorithm>
#include <map>

#include "panic.h"
#include "texture.h"

namespace ggl {

namespace {

GLint
color_type_to_pixel_format(image::pixel_type type)
{
	switch (type) {
		case image::pixel_type::GRAY:
			return GL_LUMINANCE;

		case image::pixel_type::GRAY_ALPHA:
			return GL_LUMINANCE_ALPHA;

		case image::pixel_type::RGB:
			return GL_RGB;

		case image::pixel_type::RGB_ALPHA:
			return GL_RGBA;

		default:
			panic("invalid pixel type");
	}
}

template <typename T>
static T
next_power_of_2(T n)
{
	--n;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	return n + 1;
}

} // namespace

texture::texture(const image& im)
: orig_width { im.width }, width { next_power_of_2(orig_width) }
, orig_height { im.height }, height { next_power_of_2(orig_height) }
, pixel_type { im.type }
, data(height*width*im.get_pixel_size())
, id_ { 0 }
{
	glGenTextures(1, &id_);

	const uint8_t *src = &im.data[(im.height - 1)*im.get_row_stride()];
	uint8_t *dest = &data[0];

	const size_t dest_stride = width*im.get_pixel_size();
	const size_t src_stride = im.get_row_stride();

	for (size_t i = 0; i < im.height; i++) {
		std::copy(src, src + src_stride, dest);
		src -= src_stride;
		dest += dest_stride;
	}

	const GLint format = color_type_to_pixel_format(pixel_type);

	bind();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		format,
		width, height,
		0,
		format,
		GL_UNSIGNED_BYTE,
		&data[0]);

	set_wrap_s(GL_CLAMP);
	set_wrap_t(GL_CLAMP);

	set_mag_filter(GL_LINEAR);
	set_min_filter(GL_LINEAR);

	set_env_mode(GL_MODULATE);
}

}
