#include "math_utils.h"
#include <cstdlib>
int round_float(const float & value)
{
	return static_cast<int>(value + 0.5f);
}

int floor_float(const float & value)
{
	return static_cast<int>(value);
}

float random_float(float min, float max)
{
	return map(rand(), 0, RAND_MAX, min, max);
}
