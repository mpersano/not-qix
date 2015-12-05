uniform mat4 proj_matrix;
uniform mat4 modelview_matrix;

uniform float offs;

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;

void main(void)
{
	vec3 p = position + offs*normal;
	gl_Position = proj_matrix*modelview_matrix*vec4(p, 1.);
}
