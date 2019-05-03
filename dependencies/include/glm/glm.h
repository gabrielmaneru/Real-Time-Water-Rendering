#pragma once
#include "vec2.hpp"
#include "vec3.hpp"
#include "vec4.hpp"
#include "mat2x2.hpp"
#include "mat3x3.hpp"
#include "mat4x4.hpp"
#include "gtx/norm.hpp"
#include "gtc/type_ptr.hpp"
#include "gtc/quaternion.hpp"
#include "gtx/quaternion.hpp"

using glm::vec2;
using glm::ivec2;
using glm::vec3;
using glm::vec4;
using glm::mat2;
using glm::mat3;
using glm::mat4;
using glm::quat;

template <typename T>
T lerp(const T& min, const T& max, const float& coef)
{
	return min + (max - min) * coef;
}
template <typename T>
float coef(const T& min, const T& max, const T& value)
{
	return static_cast<float>(value - min) / static_cast<float>(max - min);
}
template<typename T, typename R>
R map(T value, T in_min, T in_max, R out_min, R out_max)
{
	return lerp(out_min, out_max, coef(in_min, in_max, value));
}