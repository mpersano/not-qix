uniform sampler2D source_buffer;

uniform vec2 resolution;
uniform vec2 center;
uniform float radius;
uniform float width;
uniform float scale;

const float PI = 3.14159265;

in vec2 frag_texcoord;
out vec4 out_color;

void main(void)
{
	vec2 p = frag_texcoord*resolution;
	float r = distance(p, center);

	vec2 offs;

	if (r < radius - width || r > radius + width) {
		offs = vec2(0., 0.);
	} else {
		float d = cos((radius - r)*PI/width + .5*PI);
		offs = -d*scale*normalize(p - center);
	}

	out_color = texture2D(source_buffer, frag_texcoord + offs);
}
