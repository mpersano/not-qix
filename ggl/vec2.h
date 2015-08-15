#pragma once

template <typename T>
struct vec2
{
	vec2() : x(0), y(0) { }
	template <typename S, typename U> vec2(S x, U y) : x(x), y(y) { }
	template <typename S> vec2(const vec2<S>& v) : x(v.x), y(v.y) { }

	vec2<T>&
	operator+=(const vec2<T>& v)
	{
		x += v.x;
		y += v.y;
		return *this;
	}

	vec2<T>&
	operator-=(const vec2<T>& v)
	{
		x -= v.x;
		y -= v.y;
		return *this;
	}

	const vec2<T>
	operator-() const
	{
		return { -x, -y };
	}

	bool
	operator==(const vec2<T>& v) const
	{
		return x == v.x && y == v.y;
	}

	bool
	operator!=(const vec2<T>& v) const
	{
		return x != v.x || y != v.y;
	}

	T x, y;
};

template <typename T>
inline const vec2<T>
operator+(const vec2<T>& u, const vec2<T>& v)
{
	return { u.x + v.x, u.y + v.y };
}

template <typename T>
inline const vec2<T>
operator-(const vec2<T>& u, const vec2<T>& v)
{
	return { u.x - v.x, u.y - v.y };
}

template <typename S, typename T>
inline const vec2<T>
operator*(S s, const vec2<T>& v)
{
	return { v.x*s, v.y*s };
}

template <typename T, typename S>
inline const vec2<T>
operator*(const vec2<T>& v, S s)
{
	return { v.x*s, v.y*s };
}

template <typename T, typename S>
inline const vec2<T>
operator/(const vec2<T>& v, S s)
{
	return { v.x/s, v.y/s };
}

template <typename T>
inline float
length(const vec2<T>& v)
{
	return sqrtf(v.x*v.x + v.y*v.y);
}

template <typename T>
inline T
dot(const vec2<T>& u, const vec2<T>& v)
{
	return u.x*v.x + u.y*v.y;
}

template <typename T>
vec2<T>
normalized(const vec2<T>& v)
{
	return v/length(v);
}

using vec2s = vec2<short>;
using vec2i = vec2<int>;
using vec2f = vec2<float>;
