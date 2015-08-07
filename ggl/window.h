#pragma once

#include <vector>

namespace ggl {

enum {
	DPAD_UP = 1,
	DPAD_DOWN = 2,
	DPAD_LEFT = 4,
	DPAD_RIGHT = 8,
	DPAD_BUTTON1 = 16,
	DPAD_BUTTON2 = 32,
};

class window
{
public:
	window(int width, int height, const char *caption, bool fullscreen = false);
	virtual ~window();

	void set_dump_frames(bool b);

	void run();

	virtual void draw(float t) = 0;
	virtual void on_mouse_button_down(int button, int x, int y) { }
	virtual void on_mouse_button_up(int button) { }
	virtual void on_mouse_motion(int x, int y) { }

private:
	bool poll_events();
	void dump_frame();
	void on_key_down(int keysym);
	void on_key_up(int keysym);

protected:
	int width_, height_;
	unsigned dpad_state_;

private:
	bool dump_frames_;
	int frame_num_;
	std::vector<unsigned char> frame_data_;
};

}
