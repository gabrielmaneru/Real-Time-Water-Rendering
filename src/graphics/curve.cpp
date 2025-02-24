/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	curve.cpp
Purpose: Curve interpolation
Author: Gabriel Ma�eru - gabriel.m
- End Header --------------------------------------------------------*/
#include "curve.h"
#include <fstream>
#include <array>
#include <functional>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

float curve_base::m_epsilon = 0.01f;
int curve_base::m_forced_subdivision = 1;
bool curve_base::m_break_tangents = false;
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

std::pair<vec3,vec3> curve_base::evaluate_derivatives(float t) const
{
	// Evaluate time
	vec3 p_prev = evaluate(t - 0.05f);
	vec3 p_curr = evaluate(t);
	vec3 p_next = evaluate(t + 0.05f);

	// Compute first derivative
	vec3 front_derivative = p_curr - p_next;
	front_derivative = (glm::length2(front_derivative) > glm::epsilon<float>()) ? glm::normalize(front_derivative) : vec3(0);
	vec3 back_derivative = p_prev - p_curr;
	back_derivative = (glm::length2(back_derivative) > glm::epsilon<float>()) ? glm::normalize(back_derivative) : vec3(0);
	vec3 first_derivative = glm::normalize(front_derivative + back_derivative);

	// Compute second derivative
	vec3 second_derivative;
	float dot = glm::dot(front_derivative, back_derivative);
	if (front_derivative == vec3(0) || back_derivative == vec3(0) || (1.0 - dot) < glm::epsilon<float>())
		second_derivative = vec3(0);
	else
		second_derivative = front_derivative - back_derivative;

	return { first_derivative, second_derivative };
}

void curve_base::do_adaptive_forward_differencing()
{
	// Clear previous table
	m_length_table.clear();

	// Lambda that computes the arclenght
	std::function<float(float, float)> alen = [&](float a, float b)->float
	{
		vec3 pos_a = evaluate(a);
		vec3 pos_b = evaluate(b);
		return glm::length(pos_b - pos_a);
	};
	// Recursive lambda that subdivides the curve
	std::function<void(key_arclength&,int)> subdivide = [&](key_arclength& left, int lvl)->void
	{
		// Get previously inserted key
		const key_arclength right = m_length_table.back();

		// Compute mid time
		float mid_dt = (right.m_param_value + left.m_param_value) * 0.5f;

		// Compute arclengths
		float a_m = alen(right.m_param_value, mid_dt);
		float m_b = alen(mid_dt, left.m_param_value);

		// Computes algorithm delta
		float delta = a_m + m_b - left.m_arclength;

		// Creat mid key and update leftkey arclenght
		key_arclength mid{ mid_dt, a_m };
		left.m_arclength = m_b;

		//Check subdivision condition
		float e = m_epsilon / (float)glm::pow<int>(2, lvl);
		if (e < delta || lvl < m_forced_subdivision)
		{
			subdivide(mid,lvl+1);
			m_length_table.push_back(mid);
			subdivide(left,lvl+1);
		}
	};

	// Insert initial point
	m_length_table.push_back({ 0.0, 0.0 });

	// Iterate per segment
	for (size_t i = 0; i < m_frames.size() - point_stride; i += point_stride)
	{
		// Get keys
		keyframe right = m_frames[i];
		keyframe left = m_frames[i + point_stride];

		// Create left key
		key_arclength left_key{ left.second, alen(right.second, left.second) };

		// Start recursion preceding leftkey
		subdivide(left_key, 0);

		// Add leftkey
		m_length_table.push_back(left_key);
	}

	float acc = 0.0f;
	float max_t = m_length_table.back().m_param_value;
	for (auto& k : m_length_table)
	{
		// Normalize parameter
		k.m_param_value /= max_t;

		// Compute total distance from the start of the curve
		acc += k.m_arclength;
		k.m_arclength = acc;
	}
}

vec2 curve_base::distance_to_time(float d)const
{
	// Initalize binary search
	size_t it = 0;
	size_t step = 2;

	// Iterate
	while (true)
	{
		//Bounding condition
		if (it > m_length_table.size() - 2)
		{
			it = m_length_table.size() - 2;
			break;
		}

		// Ending condition
		if (m_length_table[it  ].m_arclength <= d
		&&  m_length_table[it+1].m_arclength >= d)
			break;

		// Update iterator
		size_t step_ = (size_t)(0.5f + m_length_table.size() / (float)step);
		if (m_length_table[it].m_arclength > d)
			it -= step_;
		else
			it += step_;

		// Update step
		step *= 2;
	}
	// Interpolate to compute the parametric value
	float param_value = map(d, m_length_table[it].m_arclength, m_length_table[it + 1].m_arclength,
		m_length_table[it].m_param_value, m_length_table[it + 1].m_param_value);

	// Ease the value
	float eased_value = m_ease->evaluate_for_x(param_value).y;
	
	// Compute the easing derivative for synchronising the animator
	float eased_prev = m_ease->evaluate_for_x(param_value - 0.01f).y;
	float eased_next = m_ease->evaluate_for_x(param_value + 0.01f).y;
	float derivative = (eased_next - eased_prev) / (0.02f);

	return { eased_value * duration(), derivative };
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
	// Get ImGui data
	const ImGuiStyle& style = ImGui::GetStyle();
	const ImGuiIO& io = ImGui::GetIO();
	ImVec2 mouse = io.MousePos;
	ImDrawList * draw_list = ImGui::GetWindowDrawList();
	ImGuiWindow * window = ImGui::GetCurrentWindow();
	const float radius = 15;

	// Initialize ImGui
	if (window->SkipItems)return;
	ImGui::Dummy(ImVec2(0, 3));
	const float avail = ImGui::GetContentRegionAvailWidth();
	ImVec2 canvas(avail, avail);
	ImRect bb(window->DC.CursorPos, ImVec2{ window->DC.CursorPos.x + canvas.x, window->DC.CursorPos.y + canvas.y });
	ImGui::ItemSize(bb);
	if (!ImGui::ItemAdd(bb, 0))return;

	// Utility functions
	std::function<vec2(vec2)> to_screen = [&](vec2 v)->vec2
	{
		return { v.x * (bb.Max.x - bb.Min.x) + bb.Min.x, (1 - v.y) * (bb.Max.y - bb.Min.y) + bb.Min.y };
	};
	std::function<vec2(vec2)> to_ndc = [&](vec2 v)->vec2
	{
		return{ (v.x - bb.Min.x) / (bb.Max.x - bb.Min.x),1 - (v.y - bb.Min.y) / (bb.Max.y - bb.Min.y) };
	};

	// Render Back Frame
	ImGui::RenderFrame(bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);
	bool hovered = ImGui::IsItemHovered();

	// Render Grid
	{
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
	}

	// Evaluate curve
	const size_t samples{ 128 };
	std::array<vec2,samples> result;
	for (size_t i = 0; i < samples; i++)
	{
		vec3 p = m_ease->evaluate(coef<size_t>(0, samples - 1,i));
		result[i] = { p.x,p.y };
	}

	// Handle grabbing
	{
		// Extract bezier points
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
		// Find closest to mouse
		float min_dist{ FLT_MAX };
		size_t idx;
		for (size_t i = 0; i < pos.size(); i++)
		{
			float d = (pos[i].x - mouse.x)*(pos[i].x - mouse.x) + (pos[i].y - mouse.y)*(pos[i].y - mouse.y);
			if (d < min_dist)
				min_dist = d, idx = i;
		}
		// Interact with it
		if (hovered && min_dist < (4 * radius * 4 * radius))
		{
			ImGui::SetTooltip("(%4.3f, %4.3f)", pos[idx].x, pos[idx].y);

			if (ImGui::IsMouseClicked(0) || ImGui::IsMouseDragging(0))
			{
				float dx = io.MouseDelta.x / canvas.x;
				float dy = -io.MouseDelta.y / canvas.y;

				// Move point
				if (idx % 3 == 0)
				{
					vec2 new_pos = to_ndc(pos[idx]);
					new_pos = { glm::clamp<float>(new_pos.x+dx, 0, 1), glm::clamp<float>(new_pos.y+dy, 0, 1) };
					m_ease->m_frames[idx].first = vec3(new_pos, 0);
					m_ease->m_frames[idx].second = new_pos.x;

					if (idx > 0)
					{
						vec2 new_tan = to_ndc(pos[idx-1]);
						new_tan = { glm::clamp<float>(new_tan.x + dx, 0, 1), glm::clamp<float>(new_tan.y + dy, 0, 1) };
						m_ease->m_frames[idx-1].first = vec3(new_tan, 0);
						m_ease->m_frames[idx-1].second = new_pos.x;
					}
					if (idx < pos.size() - 1)
					{
						vec2 new_tan = to_ndc(pos[idx + 1]);
						new_tan = { glm::clamp<float>(new_tan.x + dx, 0, 1), glm::clamp<float>(new_tan.y + dy, 0, 1) };
						m_ease->m_frames[idx+1].first = vec3(new_tan, 0);
						m_ease->m_frames[idx+1].second = new_pos.x;
					}
				}
				// Move right tangent
				else if (idx % 3 == 1)
				{
					vec2 new_tan = to_ndc(pos[idx]);
					new_tan = { glm::clamp<float>(new_tan.x + dx, 0, 1), glm::clamp<float>(new_tan.y + dy, 0, 1) };
					m_ease->m_frames[idx].first = vec3(new_tan, 0);

					if (!m_break_tangents && idx > 1)
					{
						vec2 mid_pos = to_ndc(pos[idx - 1]);
						vec2 mirror = 2.0f * (mid_pos - new_tan) + new_tan;
						mirror = { glm::clamp<float>(mirror.x, 0, 1), glm::clamp<float>(mirror.y, 0, 1) };
						m_ease->m_frames[idx - 2].first = vec3(mirror, 0);
					}
				}
				// Move left tangent
				else if (idx % 3 == 2)
				{
					vec2 new_tan = to_ndc(pos[idx]);
					new_tan = { glm::clamp<float>(new_tan.x + dx, 0, 1), glm::clamp<float>(new_tan.y + dy, 0, 1) };
					m_ease->m_frames[idx].first = vec3(new_tan, 0);

					if (!m_break_tangents && idx < pos.size() - 2)
					{
						vec2 mid_pos = to_ndc(pos[idx+1]);
						vec2 mirror = 2.0f * (mid_pos - new_tan) + new_tan;
						mirror = { glm::clamp<float>(mirror.x, 0, 1), glm::clamp<float>(mirror.y, 0, 1) };
						m_ease->m_frames[idx + 2].first = vec3(mirror, 0);
					}
				}
			}
			// Remove point
			else if (ImGui::IsMouseClicked(1) && m_ease->m_frames.size() > 4)
			{
				if((idx % 3 == 0) && idx > 2 && idx < m_ease->m_frames.size() - 3)
					m_ease->m_frames.erase(m_ease->m_frames.begin() + idx - 1, m_ease->m_frames.begin() + idx + 2);
			}
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
	{
		ImVec4 red(1.0f, 0.0f, 0.0f, 1.0f);
		ImVec4 green(0.0f, 1.0f, 0.0f, 1.0f);
		ImVec4 blue(0.0f, 0.0f, 1.0f, 1.0f);
		ImVec4 gray(0.4f, 0.4f, 0.4f, 1.0f);
		for (size_t i = 0; i < m_ease->m_frames.size(); i += 3)
		{
			// Draw right tangent
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
			// Draw left tangent
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
			// Draw point and its tangents
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
	}

	// Create new node
	if (ImGui::IsMouseDoubleClicked(0) && hovered)
	{
		vec2 m = to_ndc({ mouse.x,mouse.y });
		if (m.x > 0 && m.y > 0 && m.x < 1 && m.y < 1)
			for (size_t i = 0; i < m_ease->m_frames.size() - 3; i += 3)
				if (m_ease->m_frames[i].first.x < m.x
				&&  m_ease->m_frames[i + 3].first.x > m.x)
				{
					keyframe p{ vec3(m,0),m.x };
					keyframe t0{ vec3(m + vec2(-0.025f),0.0f),m.x };
					keyframe t1{ vec3(m + vec2(0.025f),0.0f),m.x };

					t0.first.x = glm::clamp<float>(t0.first.x, 0, 1);
					t0.first.y = glm::clamp<float>(t0.first.y, 0, 1);
					t1.first.x = glm::clamp<float>(t1.first.x, 0, 1);
					t1.first.y = glm::clamp<float>(t1.first.y, 0, 1);

					m_ease->m_frames.insert(m_ease->m_frames.begin() + i + 2, { t0,p,t1 });
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

vec3 curve_bezier::evaluate_for_x(float x) const
{
	x = glm::clamp(x, 0.0f, 1.0f);
	float it{ 0.0f };
	float step{ 2 };
	const size_t num_div{ 100 };

	// Do binary search for approximating 
	for(size_t i = 0; i < num_div; i++)
	{
		if (evaluate(it).x <= x)
			it += 1/step;
		else
			it -= 1/step;
		step *= 2;
	}
	return evaluate(it);
}
