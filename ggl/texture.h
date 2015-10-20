#pragma once

#include <ggl/gl.h>
#include <ggl/noncopyable.h>
#include <ggl/image.h>

namespace ggl {

class texture : private noncopyable
{
public:
	texture(const image& pm);
	~texture();

	void bind() const
	{ glBindTexture(GL_TEXTURE_2D, id_); }

	unsigned row_stride() const
	{ return width*pixel_size(); }

	unsigned pixel_size() const
	{ return get_pixel_size(type); }

	void load();
	void unload();

	unsigned orig_width, width;
	unsigned orig_height, height;
	pixel_type type;
	std::vector<uint8_t> data;

private:
	void set_wrap_s(GLint wrap) const;
	void set_wrap_t(GLint wrap) const;

	void set_mag_filter(GLint filter) const;
	void set_min_filter(GLint filter) const;

	void set_parameter(GLenum name, GLint value) const;
	static void set_env_mode(GLint mode);

	GLuint id_;
};

}
