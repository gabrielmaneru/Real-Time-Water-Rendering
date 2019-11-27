/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	math_utils.cpp
Purpose: Some mathematical utils
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/
#include "math_utils.h"
#include <cstdlib>
#include <graphics/renderer.h>
#include <ctime>
quat lerp(const quat& min, const quat& max, const float& coef)
{
	// Try Slerp
	float cos_theta = glm::dot(min, max);

	quat fixed_max;
	if (cos_theta < 0.0f)
	{
		fixed_max = -max;
		cos_theta = -cos_theta;
	}
	else
		fixed_max = max;

	// Avoid Slerp when cos_theta is close to 1
	if (cos_theta < 1.0f - glm::epsilon<float>())
	{
		float theta = glm::acos(cos_theta);
		return glm::normalize(  (glm::sin((1 - coef)*theta)* min + glm::sin(coef*theta)* fixed_max)
								/ glm::sin(theta));
	}

	// do NLerp
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

int ceil_float(const float & value)
{
	return static_cast<int>(glm::ceil(value));
}

float random_float(float min, float max)
{
	static bool init{ false };
	if(!init)
		srand((unsigned int)time(NULL)),init = true;
	return map(rand(), 0, RAND_MAX, min, max);
}
