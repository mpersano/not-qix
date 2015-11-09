#include "tween.h"

namespace ggl { namespace tween {

float
linear(float t)
{
	return t;
}

float
in_quadratic(float t)
{
	return t*t;
}

float
out_quadratic(float t)
{
	return -t*(t - 2.f);
}

float
in_out_quadratic(float t)
{
	return t < .5f ? 2.f*t*t : -2.f*t*(t - 2.f) - 1.f;
}

} }
