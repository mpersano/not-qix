#pragma once

#include <cstdlib>

template <typename T>
T
rand(T from, T to) // [from:to)
{
	return from + (static_cast<float>(rand())/RAND_MAX)*(to - from);
}
