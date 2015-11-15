#pragma once

template <typename T>
struct bezier
{
	bezier(const T& c0, const T& c1, const T& c2)
	: c0 { c0 }, c1 { c1 }, c2 { c2 }
	{ }

	T operator()(float u) const
	{
		const float w0 = (1 - u)*(1 - u);
		const float w1 = 2*u*(1 - u);
		const float w2 = u*u;

		return c0*w0 + c1*w1 + c2*w2;
	}

	T c0, c1, c2;
};
