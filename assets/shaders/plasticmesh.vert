uniform mat4 proj_matrix;
uniform mat4 modelview_matrix;

uniform vec3 color;

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;

out vec4 frag_color;

const vec3 light = normalize(vec3(1., 1., 1.));

void main(void)
{
	vec3 n = normalize(modelview_matrix*vec4(normal, 0.)).xyz;

	float l = dot(n, light);
	if (l < 0.)
		l = 0.;

	gl_Position = proj_matrix*modelview_matrix*vec4(position, 1.);

	// ADVANCED LIGHTING MODEL
	frag_color = vec4(vec3(.1, .1, .1) + l*color + l*l*l*vec3(1., 1., 1.), 1.);
}
