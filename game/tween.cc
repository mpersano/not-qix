#include <cmath>

#include "tween.h"

namespace tween {

float
linear(float t)
{
	return t;
}

float
quadratic(float t)
{
	return t*t;
}

float
in_cos(float t)
{
	return cosf((1.f - t)*.5f*M_PI);
}

float
out_cos(float t)
{
	return 1.f - cosf(t*.5f*M_PI);
}

// stolen from robert penner

float
in_back(float t)
{
	const float s = 1.70158;
	return t*t*((s + 1)*t - s);
}

float
out_bounce(float t)
{
	if (t < 1./2.75) {
		return 7.5625*t*t;
	} else if (t < 2./2.75) {
		t -= 1.5/2.75;
		return 7.5625*t*t + .75;
	} else if (t < 2.5/2.75) {
		t -= 2.25/2.75;
		return 7.5625*t*t + .9375;
	} else {
		t -= 2.625/2.75;
		return 7.5625*t*t + .984375;
	}
}

float
exp(float t)
{
	return -powf(2.f, -10.f*t) + 1.f;
}

} // tween
