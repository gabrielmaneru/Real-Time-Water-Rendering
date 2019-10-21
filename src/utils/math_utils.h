/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	math_utils.h
Purpose: Some mathematical utils
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/
#pragma once
#include <glm/glm.h>
template <typename T>
T lerp(const T& min, const T& max, const float& coef)
{
	return static_cast<T>(min + (max - min) * coef);
}
quat lerp(const quat& min, const quat& max, const float& coef);

template <typename T>
float coef(const T & min, const T & max, const T & value)
{
	return static_cast<float>(value - min) / static_cast<float>(max - min);
}
template<typename T, typename R>
R map(T value, T in_min, T in_max, R out_min, R out_max)
{
	return lerp(out_min, out_max, coef(in_min, in_max, value));
}
int round_float(const float & value);
int floor_float(const float & value);
float random_float(float min, float max);