#include <cstdlib>

#include "util.h"

int
rand(int from, int to)
{
	return from + rand()%(to - from);
}
