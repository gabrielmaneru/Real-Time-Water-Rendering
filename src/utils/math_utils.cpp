#include "math_utils.h"

int round_float(const float & value)
{
	return static_cast<int>(value + 0.5f);
}

int floor_float(const float & value)
{
	return static_cast<int>(value);
}
