uniform mat4 proj_modelview;

layout(location=0) in vec2 position;
layout(location=1) in vec2 texcoord0;
layout(location=2) in vec2 texcoord1;
layout(location=3) in vec4 color;

out vec2 frag_texcoord0;
out vec2 frag_texcoord1;
out vec4 frag_color;

void main(void)
{
	gl_Position = proj_modelview*vec4(position, 0., 1.);
	frag_texcoord0 = texcoord0;
	frag_texcoord1 = texcoord1;
	frag_color = color;
}
