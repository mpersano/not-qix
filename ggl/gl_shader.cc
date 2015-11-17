#include <string>
#include <vector>

#include <ggl/panic.h>
#include <ggl/gl_shader.h>

namespace ggl {

gl_shader::gl_shader(GLenum type)
: id_(glCreateShader(type))
{ }

gl_shader::~gl_shader()
{
	glDeleteShader(id_);
}

void
gl_shader::set_source(const char *source) const
{
	glShaderSource(id_, 1, &source, 0);
}

void
gl_shader::compile() const
{
	glCompileShader(id_);

	GLint status;
	glGetShaderiv(id_, GL_COMPILE_STATUS, &status);
	if (!status)
		panic("failed to compile gl_shader\n%s", get_info_log().c_str());
}

std::string
gl_shader::get_info_log() const
{
	std::string log_string;

	GLint length;
	glGetShaderiv(id_, GL_INFO_LOG_LENGTH, &length);

	if (length > 0) {
		GLint written;

		std::vector<GLchar> data(length + 1);
		glGetShaderInfoLog(id_, length, &written, &data[0]);

		log_string.assign(data.begin(), data.begin() + written);
	}

	return log_string;
}

}
