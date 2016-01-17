uniform sampler2D source_buffer;

in vec2 frag_texcoord;
out vec4 out_color;

void main(void)
{
	out_color = texture(source_buffer, frag_texcoord);
}
