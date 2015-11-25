#include <vector>

#include <ggl/panic.h>
#include <ggl/gl_check.h>
#include <ggl/gl_program.h>

namespace ggl {

gl_program::gl_program()
: id_(gl_check_r(glCreateProgram()))
{
}

gl_program::~gl_program()
{
	gl_check(glDeleteProgram(id_));
}

void
gl_program::attach(const gl_shader& shader) const
{
	gl_check(glAttachShader(id_, shader.get_id()));
}

void
gl_program::link() const
{
	gl_check(glLinkProgram(id_));
}

GLint
gl_program::get_uniform_location(const GLchar *name) const
{
	GLint rv = gl_check_r(glGetUniformLocation(id_, name));
	if (rv == -1)
		panic("get_uniform_location failed for %s\n", name);
	return rv;
}

GLint
gl_program::get_attribute_location(const GLchar *name) const
{
	GLint rv = gl_check_r(glGetAttribLocation(id_, name));
	if (rv == -1)
		fprintf(stderr, "get_attribute_location failed for %s\n", name);
	return rv;
}

void
gl_program::set_uniform_f(const GLchar *name, GLfloat v0) const
{
	gl_check(glUniform1f(get_uniform_location(name), v0));
}

void
gl_program::set_uniform_f(const GLchar *name, GLfloat v0, GLfloat v1) const
{
	gl_check(glUniform2f(get_uniform_location(name), v0, v1));
}

void
gl_program::set_uniform_f(const GLchar *name, GLfloat v0, GLfloat v1, GLfloat v2) const
{
	gl_check(glUniform3f(get_uniform_location(name), v0, v1, v2));
}

void
gl_program::set_uniform_f(const GLchar *name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) const
{
	gl_check(glUniform4f(get_uniform_location(name), v0, v1, v2, v3));
}

void
gl_program::set_uniform_i(const GLchar *name, GLint v0) const
{
	gl_check(glUniform1i(get_uniform_location(name), v0));
}

void
gl_program::set_uniform_i(const GLchar *name, GLint v0, GLint v1) const
{
	gl_check(glUniform2i(get_uniform_location(name), v0, v1));
}

void
gl_program::set_uniform_i(const GLchar *name, GLint v0, GLint v1, GLint v2) const
{
	gl_check(glUniform3i(get_uniform_location(name), v0, v1, v2));
}

void
gl_program::set_uniform_i(const GLchar *name, GLint v0, GLint v1, GLint v2, GLint v3) const
{
	gl_check(glUniform4i(get_uniform_location(name), v0, v1, v2, v3));
}

void
gl_program::set_uniform_mat4(const GLchar *name, const mat4& mat) const
{
	GLfloat gl_matrix[16] = {
		mat.m11, mat.m12, mat.m13, mat.m14,
		mat.m21, mat.m22, mat.m23, mat.m24,
		mat.m31, mat.m32, mat.m33, mat.m34,
		0, 0, 0, 1 };

	gl_check(glUniformMatrix4fv(get_uniform_location(name), 1, 1, gl_matrix));
}

void
gl_program::parameter_i(GLenum name, GLint value)
{
	gl_check(glProgramParameteri(id_, name, value));
}

void
gl_program::use() const
{
	gl_check(glUseProgram(id_));
}

std::string
gl_program::get_info_log() const
{
	std::string log_string;

	GLint length;
	gl_check(glGetProgramiv(id_, GL_INFO_LOG_LENGTH, &length));

	if (length) {
		GLint written;

		std::vector<GLchar> data(length + 1);
		gl_check(glGetProgramInfoLog(id_, length, &written, &data[0]));

		log_string.assign(data.begin(), data.begin() + written);
	}

	return log_string;
}

}
