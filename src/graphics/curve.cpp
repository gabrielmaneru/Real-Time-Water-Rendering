/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	curve.cpp
Purpose: Curve interpolation
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/
#include "curve.h"
#include <fstream>
#include <list>
#include <functional>

float curve_base::m_epsilon = 0.01f;
curve_base::curve_base(std::string path)
{
	std::string real_path = "./data/curves/" + path + ".txt";
	std::ifstream file;
	file.open(real_path);
	if (file.is_open())
	{
		std::string stream;
		file.seekg(0, std::ios::end);
		stream.reserve(file.tellg());
		file.seekg(0, std::ios::beg);
		stream.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

		// Parse the curve
		while (1)
		{
			size_t s;
			s = stream.find("KF");
			if (s > stream.size()) break;
			stream = stream.substr(s);

			s = stream.find("time");
			float time = (float)std::atof(stream.substr(s + 7, stream.find(";") - s - 7).c_str());

			vec3 value;
			s = stream.find("vals");
			stream = stream.substr(s+7);
			value.x = (float)std::atof(stream.substr(0, stream.find(",")).c_str());
			stream = stream.substr(stream.find(",") + 1);
			value.y = (float)std::atof(stream.substr(0, stream.find(",")).c_str());
			stream = stream.substr(stream.find(",") + 1);
			value.z = (float)std::atof(stream.substr(0, stream.find(",")).c_str());

			m_frames.push_back({ value, time });
		}
		file.close();
		m_name = path;
	}
}

void curve_base::do_adaptive_forward_differencing()
{
	m_length_table.clear();

	std::list<key_arclength> temp_list;

	std::function<float(float, float)> alen = [&](float a, float b)->float
	{
		vec3 pos_a = evaluate(a);
		vec3 pos_b = evaluate(b);
		return glm::length(pos_b - pos_a);
	};
	std::function<void(std::list<key_arclength>::iterator, std::list<key_arclength>::iterator)> subdivide = [&](std::list<key_arclength>::iterator a, std::list<key_arclength>::iterator b)->void
	{
		float mid_dt = (a->m_param_value + b->m_param_value) * 0.5f;
		float a_m = alen(a->m_param_value, mid_dt);
		float m_b = alen(mid_dt, b->m_param_value);
		float delta = a_m + m_b - b->m_arclength;

		auto mid = temp_list.insert(b, { mid_dt, a_m });
		b->m_arclength = m_b;

		if (delta > m_epsilon)
		{
			subdivide(a, mid);
			subdivide(mid, b);
		}
	};

	temp_list.push_back({ duration(), alen(0.0,duration())});
	auto end = temp_list.begin();

	temp_list.push_front({ 0.0, 0.0 });
	auto start = temp_list.begin();

	subdivide(start, end);

	float acc = 0.0f;
	float max_t = temp_list.back().m_param_value;
	for (auto& k : temp_list)
	{
		m_length_table.push_back({ k.m_param_value / max_t, acc + k.m_arclength });
		acc += k.m_arclength;
	}
}

float curve_base::distance_to_time(float d)const
{
	float d_clamped = fmod(d, max_distance());

	size_t it = 0;
	size_t step = 2;
	while (true)
	{
		if (it > m_length_table.size() - 2)
		{
			it = m_length_table.size() - 2;
			break;
		}

		if (m_length_table[it  ].m_arclength <= d
		&&  m_length_table[it+1].m_arclength >= d)
			break;

		size_t step_ = (size_t)(0.5f + m_length_table.size() / (float)step);
		if (m_length_table[it].m_arclength > d)
			it -= step_;
		else
			it += step_;

		step *= 2;
	}
	return map(d, m_length_table[it].m_arclength, m_length_table[it + 1].m_arclength,
	m_length_table[it].m_param_value, m_length_table[it + 1].m_param_value) * duration();
}

float curve_base::duration() const
{
	return m_frames.back().second;
}

float curve_base::max_distance() const
{
	return m_length_table.back().m_arclength;
}

curve_line::curve_line(std::string s)
	:curve_base(s)
{
	do_adaptive_forward_differencing();
}

vec3 curve_line::evaluate(float t)const
{
	// Bounding assertions
	if (t <= 0.0f || m_frames.size() == 1)
		return m_frames.front().first;
	if (t >= duration())
		return m_frames.back().first;

	// Find current frame
	for (size_t i = 0; i < m_frames.size() - 1; i++)
		if (t < m_frames[i + 1].second)
			return map(t, m_frames[i].second, m_frames[i + 1].second,
				m_frames[i].first, m_frames[i + 1].first);
	return{};
}

curve_hermite::curve_hermite(std::string s)
	:curve_base(s)
{
	do_adaptive_forward_differencing();
}

vec3 curve_hermite::evaluate(float t)const
{
	// Bounding assertions
	if (t <= 0.0f || m_frames.size() == 1)
		return m_frames.front().first;
	if (t >= duration())
		return m_frames.back().first;

	// Find current frame
	for (size_t i = 0; i < m_frames.size() - 3; i += 3)
		if (t < m_frames[i + 3].second)
		{
			
			float c = coef(m_frames[i].second, m_frames[i + 3].second, t);
			vec3 P0 = m_frames[i].first;
			vec3 P1 = m_frames[i + 3].first;
			vec3 T0 = m_frames[i + 1].first;
			vec3 T1 = m_frames[i + 2].first;

			// Apply cubic interpolation
			return (2.0f*(P0 - P1) + T0 + T1)*(c*c*c) + (3.0f*(P1 - P0) - 2.0f*T0 - T1)*(c*c) + T0 * c + P0;
		}
	return{};
}

curve_catmull::curve_catmull(std::string s)
	:curve_base(s)
{
	do_adaptive_forward_differencing();
}

vec3 curve_catmull::evaluate(float t)const
{
	// Bounding assertions
	if (t <= 0.0f || m_frames.size() == 1)
		return m_frames.front().first;
	if (t >= duration())
		return m_frames.back().first;

	// Find current frame
	for (size_t i = 0; i < m_frames.size() - 1; i++)
		if (t < m_frames[i + 1].second)
		{
			float c = coef(m_frames[i].second, m_frames[i + 1].second, t);
			vec3 P1 = m_frames[i].first;
			vec3 P2 = m_frames[i + 1].first;

			// Compute Tangents
			vec3 C0, C1;
			// Initial case
			if (i == 0)
			{
				vec3 P3 = m_frames[i + 2].first;

				C0 = 0.5f*((P2 - P1) + (P2 - P3));
				C1 = 0.5f*(P3 - P1);
			}

			// Ending case
			else if (i == m_frames.size() - 2)
			{
				vec3 P0 = m_frames[i - 1].first;

				C0 = 0.5f*(P2 - P0);
				C1 = -0.5f*((P1-P2)+(P1-P0));
			}

			// Intermediate case
			else
			{
				vec3 P0 = m_frames[i - 1].first;
				vec3 P3 = m_frames[i + 2].first;

				C0 = 0.5f*(P2 - P0);
				C1 = 0.5f*(P3 - P1);
			}

			return (2.0f*(P1 - P2) + C0 + C1)*(c*c*c) + (3.0f*(P2 - P1) - 2.0f*C0 - C1)*(c*c) + C0 * c + P1;
		}
	return{};
}

curve_bezier::curve_bezier(std::string s)
	:curve_base(s)
{
	do_adaptive_forward_differencing();
}

vec3 curve_bezier::evaluate(float t)const
{
	// Bounding assertions
	if (t <= 0.0f || m_frames.size() == 1)
		return m_frames.front().first;
	if (t >= duration())
		return m_frames.back().first;

	// Find current frame
	for (size_t i = 0; i < m_frames.size() - 3; i += 3)
		if (t < m_frames[i + 3].second)
		{
			float c = coef(m_frames[i].second, m_frames[i + 3].second, t);

			vec3 P0 = m_frames[i].first;
			vec3 C0 = m_frames[i + 1].first;
			vec3 C1 = m_frames[i + 2].first;
			vec3 P1 = m_frames[i + 3].first;

			// DeCasteljau intermediate interpolation
			// First Degree
			vec3 m = lerp(P0, C0, c);
			vec3 n = lerp(C0, C1, c);
			vec3 o = lerp(C1, P1, c);

			// Second Degree
			vec3 r = lerp(m, n, c);
			vec3 s = lerp(n, o, c);

			// Point in the curve
			return lerp(r, s, c);
		}
	return{};
}