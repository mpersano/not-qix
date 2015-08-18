#include <cmath>
#include <unistd.h>
#include <sys/time.h>

#include <memory>
#include <functional>
#include <deque>

#include <ggl/gl.h>
#include <ggl/main.h>
#include <ggl/core.h>
#include <ggl/app.h>

#include "level.h"
#include "in_game_state.h"

namespace {

const float FRAME_INTERVAL = 1.f/60;

enum class swipe { LEFT, RIGHT, UP, DOWN, NONE };

class swipe_listener
{
public:
	virtual ~swipe_listener() { }

	virtual void on_swipe(swipe s) = 0;
};

static const float MAX_SWIPE_DT = .5;
static const int MIN_SWIPE_DIST = 50;

class swipe_detector
{
public:
	swipe_detector(swipe_listener& listener);

	void on_pointer_down(int x, int y);
	void on_pointer_up(int x, int y);
	void on_pointer_motion(int x, int y);

private:
	swipe find_swipe() const;

	struct pointer_event {
		float when;
		int x, y;
	};

	std::deque<pointer_event> events_;
	swipe_listener& listener_;
};

swipe_detector::swipe_detector(swipe_listener& listener)
: listener_ { listener }
{ }

void
swipe_detector::on_pointer_down(int x, int y)
{
	events_.clear();
	events_.push_back({ ggl::g_core->now(), x, y });
}

void
swipe_detector::on_pointer_motion(int x, int y)
{
	auto t = ggl::g_core->now();

	// delete old events
	while (!events_.empty() && (t - events_.front().when) > MAX_SWIPE_DT)
		events_.pop_front();

	events_.push_back({ t, x, y });

	swipe s = find_swipe();

	if (s != swipe::NONE) {
		events_.clear();
		listener_.on_swipe(s);
	}
}

void
swipe_detector::on_pointer_up(int x, int y)
{
	events_.clear();
}

swipe
swipe_detector::find_swipe() const
{
	if (events_.size() < 2)
		return swipe::NONE;

	auto distance = [](const pointer_event& e0, const pointer_event& e1)
		{
			int dx = e1.x - e0.x;
			int dy = e1.y - e0.y;
			return sqrtf(dx*dx + dy*dy);
		};

	auto& last = events_.back();

	for (int i = events_.size() - 1; i >= 0; i--) {
		auto& first = events_[i];

		float dist = distance(first, last);

		if (dist > MIN_SWIPE_DIST) {
			float d = 0;

			for (int j = i; j < events_.size() - 1; j++)
				d += distance(events_[j], events_[j + 1]);

			if (fabs(dist - d) < .1f*dist) {
				// found swipe!

				int dx = last.x - first.x;
				int dy = last.y - first.y;

				if (fabs(dx) > fabs(dy))
					return dx > 0 ? swipe::RIGHT : swipe::LEFT;
				else
					return dy > 0 ? swipe::UP : swipe::DOWN;
			}
		}
	}

	return swipe::NONE;
}

} // (anonymous namespace)

class game_app : public ggl::app, public swipe_listener
{
public:
	game_app();

	void init(int width, int height) override;
	void update_and_render(float dt) override;

	void on_pointer_down(int x, int y) override;
	void on_pointer_up(int x, int y) override;
	void on_pointer_move(int x, int y) override;

	void on_swipe(swipe s) override;

private:
	std::unique_ptr<app_state> cur_state_;
	float update_t_;
	int width_, height_;
	swipe_detector swipe_detector_;
};

game_app::game_app()
: swipe_detector_ { *this }
{ }

void
game_app::init(int width, int height)
{
	width_ = width;
	height_ = height;

	ggl::res::load_sprite_sheet("sprites/sprites");

	init_levels();

	cur_state_.reset(new in_game_state { width, height });

	update_t_ = 0;
}

void
game_app::update_and_render(float dt)
{
	update_t_ += dt;

	while (update_t_ > FRAME_INTERVAL) {
		cur_state_->update();
		update_t_ -= FRAME_INTERVAL;
	}

	glViewport(0, 0, width_, height_);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width_, 0, height_, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	cur_state_->draw();
}

void
game_app::on_pointer_down(int x, int y)
{
	swipe_detector_.on_pointer_down(x, y);
}

void
game_app::on_pointer_up(int x, int y)
{
	swipe_detector_.on_pointer_up(x, y);
}

void
game_app::on_pointer_move(int x, int y)
{
	swipe_detector_.on_pointer_motion(x, y);
}

void
game_app::on_swipe(swipe s)
{
	// XXX
	printf("swipe: %d\n", static_cast<int>(s));
}

GGL_MAIN(game_app)
