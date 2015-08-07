#include <cstdio>
#include <cstring>
#include <cerrno>

#include <png.h>

#include "panic.h"
#include "file.h"
#include "image.h"

namespace ggl {

namespace {

image::pixel_type
to_pixel_type(png_byte png_color_type)
{
	switch (png_color_type) {
		case PNG_COLOR_TYPE_GRAY:
			return image::pixel_type::GRAY;

		case PNG_COLOR_TYPE_GRAY_ALPHA:
			return image::pixel_type::GRAY_ALPHA;

		case PNG_COLOR_TYPE_RGB:
			return image::pixel_type::RGB;

		case PNG_COLOR_TYPE_RGBA:
			return image::pixel_type::RGB_ALPHA;

		default:
			panic("invalid PNG color type: %x", png_color_type);
	}
}

} // namespace

image::image(unsigned width, unsigned height, pixel_type type)
: width(width)
, height(height)
, type(type)
, data(width*height*get_pixel_size())
{ }

unsigned
image::get_pixel_size() const
{
	switch (type) {
		case image::pixel_type::GRAY:
			return 1;

		case image::pixel_type::GRAY_ALPHA:
			return 2;

		case image::pixel_type::RGB:
			return 3;

		case image::pixel_type::RGB_ALPHA:
			return 4;
	}
}

unsigned
image::get_row_stride() const
{
	return get_pixel_size()*width;
}

image::image(const char *path)
{
	fprintf(stderr, "loading %s...\n", path);

	file in_file(path, "rb");
	if (!in_file) {
		panic("failed to open %s: %s", path, strerror(errno));
	}

	png_structp png_ptr;

	if (!(png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0))) {
		panic("png_create_read_struct");
	}

	png_infop info_ptr;

	if (!(info_ptr = png_create_info_struct(png_ptr))) {
		panic("png_create_info_struct");
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		panic("png error?");
	}

	png_init_io(png_ptr, in_file.fp);

	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, 0);

	if (png_get_bit_depth(png_ptr, info_ptr) != 8) {
		panic("invalid PNG bit depth");
	}

	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);
	type = to_pixel_type(png_get_color_type(png_ptr, info_ptr));
	data.resize(width*height*get_pixel_size());

	auto rows = png_get_rows(png_ptr, info_ptr);

	uint8_t *dest = &data[0];
	const unsigned stride = get_row_stride();

	for (unsigned i = 0; i < height; i++) {
		std::copy(rows[i], rows[i] + stride, dest);
		dest += stride;
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
}

} // ggl
