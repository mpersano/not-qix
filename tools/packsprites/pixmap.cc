#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <cassert>
#include <algorithm>

#include <png.h>

#include "pixmap.h"
#include "panic.h"

pixmap *
pixmap::load(const char *path)
{
	FILE *in;

	if ((in = fopen(path, "rb")) == 0)
		panic("failed to open `%s': %s", path, strerror(errno));

	png_structp png_ptr;
	if ((png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)) == 0)
		panic("png_create_read_struct failed");

	png_infop info_ptr;
	if ((info_ptr = png_create_info_struct(png_ptr)) == 0)
		panic("png_create_info_struct failed");

	if (setjmp(png_jmpbuf(png_ptr)))
		panic("some kind of png error");

	png_init_io(png_ptr, in);
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, 0);

	int color_type = png_get_color_type(png_ptr, info_ptr);
	int bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	if (bit_depth != 8)
		panic("invalid bit depth in PNG");

	type pixmap_type = INVALID;

	switch (color_type) {
		case PNG_COLOR_TYPE_GRAY:
			pixmap_type = GRAY;
			break;

		case PNG_COLOR_TYPE_GRAY_ALPHA:
			pixmap_type = GRAY_ALPHA;
			break;

		case PNG_COLOR_TYPE_RGB:
			pixmap_type = RGB;
			break;

		case PNG_COLOR_TYPE_RGBA:
			pixmap_type = RGB_ALPHA;
			break;

		default:
			panic("invalid color type in PNG");
	}

	const size_t width = png_get_image_width(png_ptr, info_ptr);
	const size_t height = png_get_image_height(png_ptr, info_ptr);

	const size_t stride = width*get_pixel_size(pixmap_type);

	pixmap *pm = new pixmap(width, height, pixmap_type);

	png_bytep *rows = png_get_rows(png_ptr, info_ptr);

	for (size_t i = 0; i < height; i++)
		memcpy(&pm->bits_[i*stride], rows[i], stride);

	png_destroy_read_struct(&png_ptr, &info_ptr, 0);

	fclose(in);

	return pm;
}

pixmap::pixmap(int width, int height, type pixmap_type)
: width_(width)
, height_(height)
, bits_(new uint8_t[width*height*get_pixel_size(pixmap_type)])
, type_(pixmap_type)
{
	::memset(bits_, 0, width*height*get_pixel_size());
}

pixmap::~pixmap()
{
	delete[] bits_;
}

void
pixmap::resize(size_t new_width, size_t new_height)
{
	if (new_width != width_ || new_height != height_) {
		size_t pixel_size = get_pixel_size();

		uint8_t *new_bits = new uint8_t[new_width*new_height*pixel_size];
		::memset(new_bits, 0, new_width*new_height*pixel_size);

		const int copy_height = std::min(height_, new_height);
		const int copy_width = std::min(width_, new_width);

		for (int i = 0; i < copy_height; i++) {
			uint8_t *dest = &new_bits[i*new_width*pixel_size];
			uint8_t *src = &bits_[i*width_*pixel_size];
			::memcpy(dest, src, copy_width*pixel_size);
		}

		delete[] bits_;

		width_ = new_width;
		height_ = new_height;
		bits_ = new_bits;
	}
}

#ifndef png_jmpbuf
#define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

void
pixmap::save(const char *path) const
{
	FILE *fp;

	if ((fp = fopen(path, "wb")) == NULL)
		panic("fopen %s for write failed: %s", strerror(errno));

	png_structp png_ptr;

	if ((png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL)) == NULL)
		panic("png_create_write_struct");

	png_infop info_ptr;

	if ((info_ptr = png_create_info_struct(png_ptr)) == NULL)
		panic("png_create_info_struct");

	if (setjmp(png_jmpbuf(png_ptr)))
		panic("png error");

	png_init_io(png_ptr, fp);

	png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

	int color_type;

	switch (type_) {
		case GRAY:
			color_type = PNG_COLOR_TYPE_GRAY;
			break;

		case GRAY_ALPHA:
			color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
			break;

		case RGB:
			color_type = PNG_COLOR_TYPE_RGB;
			break;

		case RGB_ALPHA:
		default:
			color_type = PNG_COLOR_TYPE_RGBA;
			break;
	}

	png_set_IHDR(
		png_ptr,
		info_ptr,
		width_, height_,
		8,
		color_type,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);

	png_write_info(png_ptr, info_ptr);

	const size_t stride = width_*get_pixel_size();

	for (size_t i = 0; i < height_; i++)
		png_write_row(png_ptr, reinterpret_cast<png_byte *>(&bits_[i*stride]));

	png_write_end(png_ptr, info_ptr);

	png_destroy_write_struct(&png_ptr, &info_ptr);

	fclose(fp);
}

size_t
pixmap::get_pixel_size() const
{
	return get_pixel_size(type_);
}

size_t
pixmap::get_pixel_size(type pixmap_type)
{
	switch (pixmap_type) {
		case GRAY:
			return 1;

		case GRAY_ALPHA:
			return 2;

		case RGB:
			return 3;

		case RGB_ALPHA:
			return 4;

		default:
			assert(0);
			return 0; // XXX
	}
}

bool
pixmap::row_is_empty(int row) const
{
	switch (type_) {
		case GRAY_ALPHA:
			{
			const uint16_t *p = reinterpret_cast<uint16_t *>(bits_) + row*width_;

			for (size_t i = 0; i < width_; i++) {
				if (p[i] >> 8)
					return false;
			}

			return true;
			}

		case RGB_ALPHA:
			{
			const uint32_t *p = reinterpret_cast<uint32_t *>(bits_) + row*width_;

			for (size_t i = 0; i < width_; i++) {
				if (p[i] >> 24)
					return false;
			}

			return true;
			}

		default:
			return false;
	}
}

bool
pixmap::column_is_empty(int col) const
{
	switch (type_) {
		case GRAY_ALPHA:
			{
			const uint16_t *p = reinterpret_cast<uint16_t *>(bits_) + col;

			for (size_t i = 0; i < height_; i++) {
				if (*p >> 8)
					return false;
				p += width_;
			}

			return true;
			}

		case RGB_ALPHA:
			{
			const uint32_t *p = reinterpret_cast<uint32_t *>(bits_) + col;

			for (size_t i = 0; i < height_; i++) {
				if (*p >> 24)
					return false;
				p += width_;
			}

			return true;
			}

		default:
			return false;
	}
}
