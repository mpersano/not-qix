#include <ggl/gl.h>
#include <ggl/gl_check.h>
#include <ggl/core.h>
#include <ggl/window.h>

namespace ggl {

window::window()
: render_target { g_core->get_viewport_width(), g_core->get_viewport_height() }
{ }

void
window::bind() const
{
	gl_check(glViewport(0, 0, width_, height_));
	gl_check(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

}
