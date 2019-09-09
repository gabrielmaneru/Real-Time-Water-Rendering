#pragma once
template <typename T>
T lerp(const T& min, const T& max, const float& coef)
{
	return min + (max - min) * coef;
}
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