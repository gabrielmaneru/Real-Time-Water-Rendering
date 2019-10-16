#pragma once
#include <utils/math_utils.h>
#include <vector>
using keyframe = std::pair<vec3, float>;
struct curve
{
	curve(std::string);
	virtual vec3 evaluate(float t)const = 0;
	float duration()const;
	std::vector<keyframe> m_frames;
};

struct curve_line : public curve
{
	curve_line(std::string s) :curve(s) {}
	vec3 evaluate(float t)const override;
};

struct curve_hermite : public curve
{
	curve_hermite(std::string s) :curve(s) {}
	vec3 evaluate(float t)const override;
};

struct curve_catmull : public curve
{
	curve_catmull(std::string s) :curve(s) {}
	vec3 evaluate(float t)const override;
};

struct curve_bezier : public curve
{
	curve_bezier(std::string s) :curve(s) {}
	vec3 evaluate(float t)const override;
};