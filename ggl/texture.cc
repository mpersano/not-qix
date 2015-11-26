#include <cassert>
#include <algorithm>
#include <map>

#include <ggl/panic.h>
#include <ggl/texture.h>
#include <ggl/gl_check.h>

namespace ggl {

namespace {

GLint
color_type_to_pixel_format(pixel_type type)
{
	switch (type) {
		case pixel_type::GRAY:
			return GL_LUMINANCE;

		case pixel_type::GRAY_ALPHA:
			return GL_LUMINANCE_ALPHA;

		case pixel_type::RGB:
			return GL_RGB;

		case pixel_type::RGB_ALPHA:
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
: orig_width { im.width }
, width { next_power_of_2(orig_width) }
, orig_height { im.height }
, height { next_power_of_2(orig_height) }
, type { im.type }
, id_ { 0 }
, data_ { new uint8_t[width*height*pixel_size()] }
{
	const uint8_t *src = &im.data[(im.height - 1)*im.row_stride()];
	uint8_t *dest = data_;

	for (unsigned i = 0; i < im.height; i++) {
		std::copy(src, src + im.row_stride(), dest);
		src -= im.row_stride();
		dest += row_stride();
	}

	load();
}

texture::texture(unsigned width, unsigned height, pixel_type type)
: orig_width { width }
, width { width }
, orig_height { height }
, height { height }
, type { type }
, id_ { 0 }
, data_ { nullptr }
{
	load();
}

texture::~texture()
{
	if (data_)
		delete[] data_;
	unload();
}

void
texture::bind() const
{
	gl_check(glBindTexture(GL_TEXTURE_2D, id_));
}

void
texture::load()
{
	assert(id_ == 0);
	gl_check(glGenTextures(1, &id_));

	bind();

	const GLint format = color_type_to_pixel_format(type);

	gl_check(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
	gl_check(glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data_));

	gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
	gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
}

void
texture::unload()
{
	gl_check(glDeleteTextures(1, &id_));
	id_ = 0;
}

}
