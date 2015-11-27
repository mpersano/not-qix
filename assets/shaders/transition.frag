uniform vec2 resolution;
uniform vec2 uv;
uniform float level;

uniform sampler2D from;
uniform sampler2D to;

in vec2 frag_texcoord;

out vec4 out_color;

const vec2 origin = vec2(-200, -100);
const float feather = 400.;
const float cell_size = 40.;

void main(void)
{
	float l0 = length(origin) - feather;
	float l1 = distance(resolution, origin);

	float lm = mix(l0, l1, level);

	vec4 c0 = texture(from, frag_texcoord);
	vec4 c1 = texture(to, frag_texcoord);

	vec2 p = (frag_texcoord/uv)*resolution;

	vec2 ps = mod(p, cell_size);

	float r = .71*cell_size*smoothstep(lm, lm + feather, distance(p - ps, origin));

	float d = distance(ps, .5*vec2(cell_size, cell_size));
	float t = smoothstep(r - 1., r, d);

	out_color = mix(c0, c1, t);
}
