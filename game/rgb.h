#pragma once

struct rgb
{
	rgb() : r { 0 }, g { 0 }, b { 0 } { }
	rgb(float r, float g, float b) : r { r }, g { g }, b { b } { }

	rgb& operator*=(const float s)
	{
		r *= s;
		g *= s;
		b *= s;
		return *this;
	}

	rgb operator*(float s) const
	{
		return rgb(*this) *= s;
	}

	friend inline
	rgb operator*(float s, const rgb& v)
	{
		return rgb(v) *= s;
	}

	rgb& operator+=(const rgb& v)
	{
		r += v.r;
		g += v.g;
		b += v.b;
		return *this;
	}

	rgb operator+(const rgb& v) const
	{
		return rgb(*this) += v;
	}

	float r, g, b;
};
