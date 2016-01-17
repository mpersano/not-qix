layout(location=0) in vec2 position;
layout(location=1) in vec2 texcoord;

out vec2 frag_texcoord;

void main()
{
	frag_texcoord = texcoord;
	gl_Position = vec4(position, 0., 1.);
}
