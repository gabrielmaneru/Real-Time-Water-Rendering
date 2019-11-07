/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	curve.h
Purpose: Curve interpolation
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/
#pragma once
#include <utils/math_utils.h>
#include <vector>
using keyframe = std::pair<vec3, float>;
struct key_arclength
{
	float m_param_value;
	float m_arclength;
};
struct curve_bezier;
struct curve_base
{
	curve_base(std::string);
	virtual vec3 evaluate(float t)const = 0;
	std::pair<vec3,vec3> evaluate_derivatives(float t)const;
	void do_adaptive_forward_differencing();
	float distance_to_time(float d)const;
	float duration()const;
	float max_distance()const;
	void draw_easing();

	static float m_epsilon;
	static int m_forced_subdivision;
	static bool m_break_tangents;
	std::vector<keyframe> m_frames;
	std::vector<key_arclength> m_length_table;
	std::string m_name;
	size_t point_stride;
	mutable curve_bezier* m_ease{nullptr};
};

struct curve_line : public curve_base
{
	curve_line(std::string s);
	vec3 evaluate(float t)const override;
};

struct curve_hermite : public curve_base
{
	curve_hermite(std::string s);
	vec3 evaluate(float t)const override;
};

struct curve_catmull : public curve_base
{
	curve_catmull(std::string s);
	vec3 evaluate(float t)const override;
};

struct curve_bezier : public curve_base
{
	curve_bezier(std::string s);
	curve_bezier(vec4 simple);
	vec3 evaluate(float t)const override;
	vec3 evaluate_for_x(float x)const;
};