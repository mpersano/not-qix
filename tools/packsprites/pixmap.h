#pragma once

#include <stdint.h>

struct margins {
	margins(int left, int right, int top, int bottom)
	: left_(left), right_(right), top_(top), bottom_(bottom)
	{ }

	int left_, right_, top_, bottom_;
};

class pixmap
{
public:
	enum type { GRAY, GRAY_ALPHA, RGB, RGB_ALPHA, INVALID };

	pixmap(int width, int height, type pixmap_type);

	virtual ~pixmap();

	size_t get_width() const
	{ return width_; }

	size_t get_height() const
	{ return height_; }

	const uint8_t *get_bits() const
	{ return bits_; }

	uint8_t *get_bits()
	{ return bits_; }

	type get_type() const
	{ return type_; }

	size_t get_pixel_size() const;

	static size_t get_pixel_size(type pixmap_type);

	void resize(size_t new_width, size_t new_height);

	// returns cropped left/top margins
	margins trim();

	static pixmap *load(const char *path);

	void save(const char *path) const;

protected:
	bool row_is_empty(int row) const;
	bool column_is_empty(int col) const;

	size_t width_;
	size_t height_;
	uint8_t *bits_;
	type type_;

private:
	pixmap(const pixmap&); // disable copy ctor
	pixmap& operator=(const pixmap&); // disable assignment
};
