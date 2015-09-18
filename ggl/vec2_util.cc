#include <ggl/vec2_util.h>

vec2f
seg_closest_point(const vec2f& v0, const vec2f& v1, const vec2f& p)
{
	vec2f d = v1 - v0;
	vec2f v = normalized(v1 - v0);

	float t = dot(p - v0, v);

	if (t < 0)
		return v0;
	else if (t > length(d))
		return v1;
	else
		return v0 + t*v;
}
