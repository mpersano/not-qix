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

	void draw(const ggl::framebuffer& source, const ggl::render_target& dest) const override;
};

class dynamic_post_filter : public post_filter
{
public:
	dynamic_post_filter(const char *program);

	virtual bool update() = 0;
};

class ripple_filter : public dynamic_post_filter
{
public:
	ripple_filter(float width, const vec2f& center, float speed, int ttl);

	void draw(const ggl::framebuffer& source, const ggl::render_target& dest) const override;
	bool update() override;

private:
	float width_;
	vec2f center_;
	float radius_;
	float speed_;
	int tics_;
	int ttl_;
};
