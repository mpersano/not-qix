uniform sampler2D tex0;
uniform sampler2D tex1;

in vec2 frag_texcoord0;
in vec2 frag_texcoord1;
in vec4 frag_color;

out vec4 out_color;

void main(void)
{
	vec4 c0 = texture(tex0, frag_texcoord0);
	vec4 c1 = texture(tex1, frag_texcoord1);
	out_color = vec4(c0.rgb + c1.rgb, c0.a)*frag_color;
}
