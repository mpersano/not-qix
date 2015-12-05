#include <cmath>

#include <ggl/gl.h>
#include <ggl/gl_check.h>
#include <ggl/render.h>
#include <ggl/vec2.h>
#include <ggl/resources.h>
#include <ggl/program.h>
#include <ggl/mesh.h>

#include "fake3d.h"

// XXX ugly test code that will be removed eventually. fake3d mesh rendering should be moved to ggl::render somehow

namespace {

const float FOV = 30.f;
const float Z_NEAR = 1.f;
const float Z_FAR = 1000.f;

GLfloat proj_matrix[16];

void
initialize_proj_matrix()
{
	const auto viewport = ggl::render::get_viewport();

	const auto viewport_width = viewport.max.x - viewport.min.x;
	const auto viewport_height = viewport.max.y - viewport.min.y;

	const float aspect = static_cast<float>(viewport_width)/viewport_height;

	const float fovy_rad = FOV*M_PI/180.;

	const float f = 1.f/tanf(.5f*fovy_rad);

	proj_matrix[0] = f/aspect;
	proj_matrix[4] = 0;
	proj_matrix[8] = 0;
	proj_matrix[12] = 0;

	proj_matrix[1] = 0;
	proj_matrix[5] = f;
	proj_matrix[9] = 1;
	proj_matrix[13] = 0;

	proj_matrix[2] = 0;
	proj_matrix[6] = 0;
	proj_matrix[10] = (Z_FAR + Z_NEAR)/(Z_NEAR - Z_FAR);
	proj_matrix[14] = (2.*Z_FAR*Z_NEAR)/(Z_NEAR - Z_FAR);

	proj_matrix[3] = 0;
	proj_matrix[7] = 0;
	proj_matrix[11] = -1;
	proj_matrix[15] = 0;

	auto prog_plastic = ggl::res::get_program("plasticmesh");
	prog_plastic->use();
	prog_plastic->set_uniform_mat4("proj_matrix", proj_matrix);

	auto prog_outline = ggl::res::get_program("outline");
	prog_outline->use();
	prog_outline->set_uniform_mat4("proj_matrix", proj_matrix);
}

mat4
get_translation(const vec2f& pos)
{
	const auto viewport = ggl::render::get_viewport();

	const auto viewport_width = viewport.max.x - viewport.min.x;
	const auto viewport_height = viewport.max.y - viewport.min.y;

	const float PLANE_Z = -50.f;

	const vec2f po = pos - viewport.min;

#if 0
	float xl = -(f/aspect)*pos.x/pos.z;
	float yl = -f*pos.y/pos.z;

	float xs = (.5f*xl + .5f)*width_;
	float ys = (.5f*yl + .5f)*height_;
#endif

	const float xl = 2.f*(po.x/viewport_width - .5f);
	const float yl = 2.f*(po.y/viewport_height - .5f);

	// XXX why do we need to double here?!!!
	const float x = 2.f*xl*PLANE_Z/-proj_matrix[0];
	const float y = 2.f*yl*PLANE_Z/-proj_matrix[5];

	return mat4::translation(x, y, PLANE_Z);
}

} // (anonymous namespace)

void
draw_mesh_prepare()
{
	initialize_proj_matrix();
}

void
draw_mesh(const ggl::mesh *mesh, const vec2f& pos, const mat4& transform)
{
	auto prog = ggl::res::get_program("plasticmesh");
	prog->use();
	prog->set_uniform_mat4("modelview_matrix", get_translation(pos)*transform);

	gl_check(glEnable(GL_CULL_FACE));
	gl_check(glFrontFace(GL_CCW));

	mesh->draw();

	gl_check(glDisable(GL_CULL_FACE));
}

void
draw_mesh_outline(const ggl::mesh *mesh, const vec2f& pos, const mat4& transform, float offset)
{
	auto prog = ggl::res::get_program("outline");
	prog->use();
	prog->set_uniform_mat4("modelview_matrix", get_translation(pos)*transform);
	prog->set_uniform_f("offs", offset);

	gl_check(glEnable(GL_CULL_FACE));
	gl_check(glFrontFace(GL_CW));

	mesh->draw();

	gl_check(glDisable(GL_CULL_FACE));
}
