#include <cmath>

#include "tween.h"

float
linear_tween(float t)
{
	return t;
}

float
quadratic_tween(float t)
{
	return t*t;
}

float cos_tween(float t)
{
	return 1.f - (.5f + .5f*cosf(t*M_PI));
}

// stolen from robert penner

float
in_back_tween(float t)
{
	const float s = 1.70158;
	return t*t*((s + 1)*t - s);
}

float
out_bounce_tween(float t)
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
