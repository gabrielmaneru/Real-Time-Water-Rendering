#pragma once
#include <utils/math_utils.h>
#include <vector>
using keyframe = std::pair<vec3, float>;
struct curve_base
{
	curve_base(std::string);
	virtual vec3 evaluate(float t)const = 0;
	float duration()const;
	std::vector<keyframe> m_frames;
};

struct curve_line : public curve_base
{
	curve_line(std::string s) :curve_base(s) {}
	vec3 evaluate(float t)const override;
};

struct curve_hermite : public curve_base
{
	curve_hermite(std::string s) :curve_base(s) {}
	vec3 evaluate(float t)const override;
};

struct curve_catmull : public curve_base
{
	curve_catmull(std::string s) :curve_base(s) {}
	vec3 evaluate(float t)const override;
};

struct curve_bezier : public curve_base
{
	curve_bezier(std::string s) :curve_base(s) {}
	vec3 evaluate(float t)const override;
};