#include <vector>

#include <ggl/core.h>
#include <ggl/asset.h>
#include <ggl/panic.h>
#include <ggl/gl_check.h>
#include <ggl/gl_program.h>

namespace ggl {

namespace {

struct shader : private noncopyable
{
public:
	// type: GL_FRAGMENT_SHADER or GL_VERTEX_SHADER
	shader(GLenum type);
	~shader();

	void set_source(const char *source) const;

	void compile() const;

	std::string get_info_log() const;

	GLuint id;
};

shader::shader(GLenum type)
: id { gl_check_r(glCreateShader(type)) }
{ }

shader::~shader()
{
	gl_check(glDeleteShader(id));
}

void
shader::set_source(const char *source) const
{
	const char *sources[] = {
#ifdef ANDROID
		"#version 300 es\n",
#else
		"#version 430 core\n",
#endif
		source };

	gl_check(glShaderSource(id, 2, sources, 0));
}

void
shader::compile() const
{
	gl_check(glCompileShader(id));

	GLint status;
	gl_check(glGetShaderiv(id, GL_COMPILE_STATUS, &status));

	if (!status)
		panic("failed to compile shader\n%s", get_info_log().c_str());
}

std::string
shader::get_info_log() const
{
	std::string log_string;

	GLint length;
	gl_check(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));

	if (length > 0) {
		GLint written;

		std::vector<GLchar> data(length + 1);
		glGetShaderInfoLog(id, length, &written, &data[0]);

		log_string.assign(data.begin(), data.begin() + written);
	}

	return log_string;
}

} // (anonymous namespace)

gl_program::gl_program(const std::string& vp_path, const std::string& fp_path)
: vp_path_ { vp_path }
, fp_path_ { fp_path }
{
	load();
}

gl_program::~gl_program()
{
	unload();
}

void
gl_program::load()
{
	id_ = gl_check_r(glCreateProgram());

	auto attach_shader = [this](GLenum type, const std::string& path)
		{
			shader s { type };

			auto data = g_core->get_asset(path)->read_all();
			data.push_back('\0');

			s.set_source(&data[0]);
			s.compile();

			gl_check(glAttachShader(id_, s.id));
		};

	attach_shader(GL_VERTEX_SHADER, vp_path_);
	attach_shader(GL_FRAGMENT_SHADER, fp_path_);

	gl_check(glLinkProgram(id_));

	GLint status;
	gl_check(glGetProgramiv(id_, GL_LINK_STATUS, &status));

	if (!status)
		panic("failed to link shader\n%s", get_info_log().c_str());
}

void
gl_program::unload()
{
	gl_check(glDeleteProgram(id_));
	id_ = 0;
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
