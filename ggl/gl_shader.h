#pragma once

#include <string>

#include <ggl/gl.h>
#include <ggl/noncopyable.h>

namespace ggl {

class gl_shader : private noncopyable
{
public:
	// type: GL_FRAGMENT_SHADER or GL_VERTEX_SHADER
	gl_shader(GLenum type);
	~gl_shader();

	void set_source(const char *source) const;

	void compile() const;

	const GLuint get_id() const
	{ return id_; }

	std::string get_info_log() const;

private:
	GLuint id_;
};

}
