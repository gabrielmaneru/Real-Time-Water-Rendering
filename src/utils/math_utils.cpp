#include "math_utils.h"
#include <cstdlib>
#include <ctime>
quat lerp(const quat& min, const quat& max, const float& coef)
{
	return glm::normalize(glm::slerp(min, max, static_cast<float>(coef)));
}
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
	static bool init{ false };
	if(!init)
		srand(time(NULL)),init = true;
	return map(rand(), 0, RAND_MAX, min, max);
}
