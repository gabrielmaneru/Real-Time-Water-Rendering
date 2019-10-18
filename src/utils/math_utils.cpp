#include "math_utils.h"
#include <cstdlib>
#include <graphics/renderer.h>
#include <ctime>
quat lerp(const quat& min, const quat& max, const float& coef)
{
	if (renderer->m_render_options.interpolate_slerp)
		return glm::normalize(glm::slerp(min, max, static_cast<float>(coef)));
	else // do NLerp
		return glm::normalize(min + (max - min) * coef);
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
		srand((unsigned int)time(NULL)),init = true;
	return map(rand(), 0, RAND_MAX, min, max);
}
