#pragma once

namespace ggl {
class framebuffer;
class render_target;
class program;
}

class post_filter
{
public:
	post_filter(const char *program);
	virtual ~post_filter() { }

	virtual void draw(const ggl::framebuffer& source, const ggl::render_target& dest) const = 0;

protected:
	const ggl::program *program_;
};

class passthru_filter : public post_filter
{
public:
	passthru_filter();

	void draw(const ggl::framebuffer& source, const ggl::render_target& dest) const;
};

class ripple_filter : public post_filter
{
public:
	ripple_filter();

	void set_params(float width, const vec2f& center, float radius) const;

	void draw(const ggl::framebuffer& source, const ggl::render_target& dest) const;
};
