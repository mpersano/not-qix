#pragma once

#include <cstdlib>

template <typename T>
T
rand(T from, T to) // [from:to)
{
	return from + rand()%(to - from);
}
