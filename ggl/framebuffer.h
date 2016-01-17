#pragma once

#include <ggl/gl.h>
#include <ggl/texture.h>
#include <ggl/render_target.h>

namespace ggl {

class texture;

class framebuffer : public render_target
{
public:
	framebuffer(int width, int height);
	~framebuffer();

	void bind() const override;
	static void unbind();

	void bind_texture() const;

private:
	void init_texture();

	GLuint texture_id_;
	GLuint fbo_id_;
};

}
