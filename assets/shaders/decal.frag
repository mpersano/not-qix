uniform sampler2D tex;

in vec2 frag_texcoord;

out vec4 out_color;

void main(void)
{
	out_color = texture(tex, frag_texcoord);
}
