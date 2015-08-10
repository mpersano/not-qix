#include <ggl/window.h>

namespace ggl {

window::window(int width, int height)
: width_ { width }
, height_ { height }
, dpad_state_ { 0 }
{ }

window::~window()
{ }

}
