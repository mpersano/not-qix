#pragma once

#include <string>

#include <ggl/noncopyable.h>
#include <ggl/gl.h>

#include <ggl/vec3.h>

namespace ggl {

class program : private noncopyable
{
public:
	program(const std::string& vp_path, const std::string& fp_path);
	~program();

	GLint get_uniform_location(const GLchar *name) const;
	GLint get_attribute_location(const GLchar *name) const;

	void set_uniform_f(const GLchar *name, GLfloat v0) const;
	void set_uniform_f(const GLchar *name, GLfloat v0, GLfloat v1) const;
	void set_uniform_f(const GLchar *name, GLfloat v0, GLfloat v1, GLfloat v2) const;
	void set_uniform_f(const GLchar *name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) const;

	void set_uniform_i(const GLchar *name, GLint v0) const;
	void set_uniform_i(const GLchar *name, GLint v0, GLint v1) const;
	void set_uniform_i(const GLchar *name, GLint v0, GLint v1, GLint v2) const;
	void set_uniform_i(const GLchar *name, GLint v0, GLint v1, GLint v2, GLint v3) const;

	void set_uniform_mat4(const GLchar *name, const mat4& mat) const;
	void set_uniform_mat4(const GLchar *name, const GLfloat *mat) const;

	void parameter_i(GLenum name, GLint value);

	void use() const;

	void load();
	void unload();

private:
	std::string get_info_log() const;

	GLuint id_;
	std::string vp_path_;
	std::string fp_path_;
};

}
