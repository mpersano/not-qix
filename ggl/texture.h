#pragma once

#include <ggl/gl.h>
#include <ggl/noncopyable.h>
#include <ggl/image.h>

namespace ggl {

class texture : private noncopyable
{
public:
	texture(unsigned width, unsigned height, pixel_type type);
	texture(const image& pm);
	~texture();

	void bind() const;

	unsigned row_stride() const
	{ return width*pixel_size(); }

	unsigned pixel_size() const
	{ return get_pixel_size(type); }

	void load();
	void unload();

	unsigned orig_width, width;
	unsigned orig_height, height;
	pixel_type type;

private:
	GLuint id_;
	uint8_t *data_;

	friend class framebuffer;
};

}
