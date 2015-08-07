#pragma once

template <typename T>
struct vec2
{
	vec2() : x(0), y(0) { }
	template <typename S, typename U> vec2(S x, U y) : x(x), y(y) { }
	template <typename S> vec2(const vec2<S>& v) : x(v.x), y(v.y) { }

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
inline bool
operator==(const vec2<T>& u, const vec2<T>& v)
{
	return u.x == v.x && u.y == v.y;
}

template <typename T>
inline float
length(const vec2<T>& v)
{
	return sqrtf(v.x*v.x + v.y*v.y);
}
