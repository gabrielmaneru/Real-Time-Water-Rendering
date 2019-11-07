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
#include <array>
#include <functional>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

float curve_base::m_epsilon = 0.01f;
int curve_base::m_forced_subdivision = 1;
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

	std::function<float(float, float)> alen = [&](float a, float b)->float
	{
		vec3 pos_a = evaluate(a);
		vec3 pos_b = evaluate(b);
		return glm::length(pos_b - pos_a);
	};
	std::function<void(key_arclength&,int)> subdivide = [&](key_arclength& left, int lvl)->void
	{
		const key_arclength right = m_length_table.back();
		float mid_dt = (right.m_param_value + left.m_param_value) * 0.5f;
		float a_m = alen(right.m_param_value, mid_dt);
		float m_b = alen(mid_dt, left.m_param_value);
		float delta = a_m + m_b - left.m_arclength;

		key_arclength mid{ mid_dt, a_m };
		left.m_arclength = m_b;
		if (m_epsilon < delta || lvl < m_forced_subdivision)
		{
			subdivide(mid,lvl+1);
			m_length_table.push_back(mid);
			subdivide(left,lvl+1);
		}
	};

	m_length_table.push_back({ 0.0, 0.0 });
	for (size_t i = 0; i < m_frames.size() - point_stride; i += point_stride)
	{
		keyframe right = m_frames[i];
		keyframe left = m_frames[i + point_stride];
		key_arclength left_key{ left.second, alen(right.second, left.second) };

		subdivide(left_key, 0);
		m_length_table.push_back(left_key);
	}

	float acc = 0.0f;
	float max_t = m_length_table.back().m_param_value;
	for (auto& k : m_length_table)
	{
		k.m_param_value /= max_t;
		acc += k.m_arclength;
		k.m_arclength = acc;
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

void curve_base::draw_easing()
{
	const char * label = "Ease";
	const ImGuiStyle& style = ImGui::GetStyle();
	const ImGuiIO& io = ImGui::GetIO();
	ImDrawList * draw_list = ImGui::GetWindowDrawList();
	ImGuiWindow * window = ImGui::GetCurrentWindow();
	if (window->SkipItems)return;

	ImGui::Dummy(ImVec2(0, 3));

	const float avail = ImGui::GetContentRegionAvailWidth();
	ImVec2 canvas(avail, avail);
	
	ImRect bb(window->DC.CursorPos, ImVec2{ window->DC.CursorPos.x + canvas.x, window->DC.CursorPos.y + canvas.y });
	ImGui::ItemSize(bb);
	if (!ImGui::ItemAdd(bb, 0))return;
	
	const ImGuiID id = window->GetID(label);

	// Render Back
	ImGui::RenderFrame(bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);
	bool hovered = ImGui::IsItemHovered();

	// Render Grid
	draw_list->AddLine(
		ImVec2(bb.Min.x + canvas.x * .25f, bb.Min.y),
		ImVec2(bb.Min.x + canvas.x * .25f, bb.Max.y),
		ImGui::GetColorU32(ImGuiCol_TextDisabled));
	draw_list->AddLine(
		ImVec2(bb.Min.x + canvas.x * .5f, bb.Min.y),
		ImVec2(bb.Min.x + canvas.x * .5f, bb.Max.y),
		ImGui::GetColorU32(ImGuiCol_TextDisabled));
	draw_list->AddLine(
		ImVec2(bb.Min.x + canvas.x * .75f, bb.Min.y),
		ImVec2(bb.Min.x + canvas.x * .75f, bb.Max.y),
		ImGui::GetColorU32(ImGuiCol_TextDisabled));
	draw_list->AddLine(
		ImVec2(bb.Min.x, bb.Min.y + canvas.y * .25f),
		ImVec2(bb.Max.x, bb.Min.y + canvas.y * .25f),
		ImGui::GetColorU32(ImGuiCol_TextDisabled));
	draw_list->AddLine(
		ImVec2(bb.Min.x, bb.Min.y + canvas.y * .5f),
		ImVec2(bb.Max.x, bb.Min.y + canvas.y * .5f),
		ImGui::GetColorU32(ImGuiCol_TextDisabled));
	draw_list->AddLine(
		ImVec2(bb.Min.x, bb.Min.y + canvas.y * .75f),
		ImVec2(bb.Max.x, bb.Min.y + canvas.y * .75f),
		ImGui::GetColorU32(ImGuiCol_TextDisabled));

	// Evaluate curve
	const size_t samples{ 128 };
	std::array<vec2,samples> result;
	for (size_t i = 0; i < samples; i++)
	{
		vec3 p = m_ease->evaluate(coef<size_t>(0, samples - 1,i));
		result[i] = { p.x,p.y };
	}

	// Handle grabbing
	std::function<vec2(vec2)> to_screen = [&](vec2 v)->vec2
	{
		return { v.x * (bb.Max.x - bb.Min.x) + bb.Min.x, (1 - v.y) * (bb.Max.y - bb.Min.y) + bb.Min.y };
	};
	std::function<vec2(vec2)> to_ndc = [&](vec2 v)->vec2
	{
		return{ (v.x - bb.Min.x) / (bb.Max.x - bb.Min.x),1 - (v.y - bb.Min.y) / (bb.Max.y - bb.Min.y) };
	};
	const float radius = 15;
	ImVec2 mouse = io.MousePos;
	std::vector<vec2>pos;
	std::vector<float>distsq;
	for (size_t i = 0; i < m_ease->m_frames.size(); i+=3)
	{
		if (i == 0)
		{
			vec2 p0 = m_ease->m_frames[0].first;
			pos.push_back(to_screen(p0));

			vec2 t1 = m_ease->m_frames[1].first;
			pos.push_back(to_screen(t1));
		}
		else if (i == m_ease->m_frames.size() - 1)
		{
			vec2 t0 = m_ease->m_frames[i-1].first;
			pos.push_back(to_screen(t0));

			vec2 p1 = m_ease->m_frames[i].first;
			pos.push_back(to_screen(p1));
		}
		else
		{
			vec2 t0 = m_ease->m_frames[i - 1].first;
			pos.push_back(to_screen(t0));

			vec2 p1 = m_ease->m_frames[i].first;
			pos.push_back(to_screen(p1));

			vec2 t2 = m_ease->m_frames[i + 1].first;
			pos.push_back(to_screen(t2));
		}
	}
	float min_dist{ FLT_MAX };
	size_t idx;
	for (size_t i = 0; i < pos.size(); i++)
	{
		float d = (pos[i].x - mouse.x)*(pos[i].x - mouse.x) + (pos[i].y - mouse.y)*(pos[i].y - mouse.y);
		if (d < min_dist)
			min_dist = d, idx = i;
	}
	if (min_dist < (4 * radius * 4 * radius))
	{
		ImGui::SetTooltip("(%4.3f, %4.3f)", pos[idx].x, pos[idx].y);

		if (ImGui::IsMouseClicked(0) || ImGui::IsMouseDragging(0))
		{
			float dx = io.MouseDelta.x / canvas.x;
			float dy = -io.MouseDelta.y / canvas.y;
			m_ease->m_frames[idx].first = vec3(
				glm::clamp<float>(m_ease->m_frames[idx].first.x+dx,0,1),
				glm::clamp<float>(m_ease->m_frames[idx].first.y + dy, 0, 1),
				0);
		}
	}

	// Draw curve
	ImColor color{ style.Colors[ImGuiCol_PlotLines] };
	for (size_t i = 0; i < samples-1; i++)
	{
		vec2 p_screen = to_screen(result[i]);
		vec2 q_screen = to_screen(result[i+1]);
		draw_list->AddLine({ p_screen.x,p_screen.y }, { q_screen.x,q_screen.y }, color, 4);
	}

	// Draw grabbers
	ImVec4 red(1.0f, 0.0f, 0.0f, 1.0f);
	ImVec4 green(0.0f, 1.0f, 0.0f, 1.0f);
	ImVec4 blue(0.0f, 0.0f, 1.0f, 1.0f);
	ImVec4 gray(0.4f, 0.4f, 0.4f, 1.0f);
	for (size_t i = 0; i < m_ease->m_frames.size(); i += 3)
	{
		if (i == 0)
		{
			vec2 p0 = m_ease->m_frames[0].first;
			vec2 t1 = m_ease->m_frames[1].first;
			p0 = to_screen(p0);
			t1 = to_screen(t1);

			draw_list->AddLine({p0.x,p0.y}, { t1.x, t1.y }, ImColor(gray), 4);
			draw_list->AddCircleFilled({ t1.x,t1.y }, radius, ImColor(gray));
			draw_list->AddCircleFilled({ t1.x,t1.y }, radius *3.0f / 4.0f, ImColor(blue));
		}
		else if (i == m_ease->m_frames.size() - 1)
		{
			vec2 t0 = m_ease->m_frames[i - 1].first;
			vec2 p1 = m_ease->m_frames[i].first;
			t0 = to_screen(t0);
			p1 = to_screen(p1);

			draw_list->AddLine({ p1.x,p1.y }, { t0.x, t0.y }, ImColor(gray), 4);
			draw_list->AddCircleFilled({ t0.x,t0.y }, radius, ImColor(gray));
			draw_list->AddCircleFilled({ t0.x,t0.y }, radius *3.0f / 4.0f, ImColor(red));
		}
		else
		{
			vec2 t0 = m_ease->m_frames[i - 1].first;
			vec2 p1 = m_ease->m_frames[i].first;
			vec2 t2 = m_ease->m_frames[i + 1].first;
			t0 = to_screen(t0);
			p1 = to_screen(p1);
			t2 = to_screen(t2);

			draw_list->AddLine({ p1.x,p1.y }, { t0.x, t0.y }, ImColor(gray), 4);
			draw_list->AddLine({ p1.x,p1.y }, { t2.x, t2.y }, ImColor(gray), 4);
			draw_list->AddCircleFilled({ t0.x,t0.y }, radius, ImColor(gray));
			draw_list->AddCircleFilled({ t0.x,t0.y }, radius *3.0f / 4.0f, ImColor(blue));
			draw_list->AddCircleFilled({ t2.x,t2.y }, radius, ImColor(gray));
			draw_list->AddCircleFilled({ t2.x,t2.y }, radius *3.0f / 4.0f, ImColor(red));
			draw_list->AddCircleFilled({ p1.x,p1.y }, radius, ImColor(gray));
			draw_list->AddCircleFilled({ p1.x,p1.y }, radius *3.0f / 4.0f, ImColor(green));
		}
	}

	// Create new node
	if (ImGui::IsMouseDoubleClicked(0) && hovered)
	{
		vec2 m = to_ndc({ mouse.x,mouse.y });
		if (m.x > 0 && m.y > 0 && m.x < 1 && m.y < 1)
		{
			for (size_t i = 0; i < m_ease->m_frames.size() - 3; i += 3)
			{
				if (m_ease->m_frames[i].first.x < m.x
				&&  m_ease->m_frames[i + 3].first.x > m.x)
				{
					m_ease->m_frames.insert(eeg)
				}
			}
		}
	}
}

curve_line::curve_line(std::string s)
	:curve_base(s)
{
	point_stride = 1;
	do_adaptive_forward_differencing();
	m_ease = new curve_bezier(vec4{ .25f,.25f, .75f, .75 });
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
	point_stride = 3;
	do_adaptive_forward_differencing();
	m_ease = new curve_bezier(vec4{ .25f,.25f, .75f, .75 });
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
	point_stride = 1;
	do_adaptive_forward_differencing();
	m_ease = new curve_bezier(vec4{ .25f,.25f, .75f, .75 });
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
	point_stride = 3;
	do_adaptive_forward_differencing();
	m_ease = new curve_bezier(vec4{ .25f,.25f, .75f, .75 });
}

curve_bezier::curve_bezier(vec4 simple)
	:curve_base("")
{
	point_stride = 3;
	m_frames.push_back({ { 0,0,0 }, 0 });
	m_frames.push_back({ { simple.x,simple.y,0 }, 0 });
	m_frames.push_back({ { simple.z,simple.w,0 }, 1 });
	m_frames.push_back({ { 1,1,0 }, 1 });
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