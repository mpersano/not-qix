#pragma once

template <typename T>
struct rgb
{
	rgb<T>&
	operator+=(const rgb<T>& o)
	{
		r += o.r;
		g += o.g;
		b += o.b;

		return o;
	}

	const rgb<T>
	operator+(const rgb<T>& o) const
	{
		return rgb<T> { r + o.r, g + o.g, b + o.b };
	}

	rgb<T>&
	operator-=(const rgb<T>& o)
	{
		r -= o.r;
		g -= o.g;
		b -= o.b;

		return o;
	}

	const rgb<T>
	operator-(const rgb<T>& o) const
	{
		return rgb<T> { r - o.r, g - o.g, b - o.b };
	}

	template <typename S>
	const rgb<T>
	operator*(S s) const
	{
		return rgb<T> { static_cast<T>(r*s), static_cast<T>(g*s), static_cast<T>(b*s) };
	}

	T r, g, b;
};

template <typename S, typename T>
const rgb<T>
operator*(S s, const rgb<T>& c)
{
	return rgb<T> { static_cast<T>(c.r*s), static_cast<T>(c.g*s), static_cast<T>(c.b*s) };
}
