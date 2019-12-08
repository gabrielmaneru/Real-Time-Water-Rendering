#include "ik_bone.h"
#include <graphics/renderer.h>
#include <imgui/imgui.h>
#include <numeric>
#include <functional>
#include <imgui/ImGuizmo.h>

mat4 ik_bone::get_model()const
{
	mat4 model{ 1.0f };
	if (m_parent)
	{
		vec3 real_pos = vec3(m_parent->get_model() * vec4(1, 0, 0, 1));
		model = glm::translate(mat4(1.0f), real_pos);
	}
	model *= get_concat_rotation();
	return glm::scale(model, vec3(m_length));
}
void ik_bone::set_model(const mat4& m)
{
	mat4 rot = m;
	if (m_parent)
		rot = glm::inverse(m_parent->get_concat_rotation())*m;

	float t[3], r[3], s[3];
	ImGuizmo::DecomposeMatrixToComponents(&rot[0][0], t, r, s);
	m_rotation = normalize(quat(radians(vec3{ r[0], r[1], r[2] })));
}
mat4 ik_bone::get_concat_rotation()const
{
	if (m_parent)
		return m_parent->get_concat_rotation() * glm::mat4_cast(m_rotation);
	return glm::mat4_cast(m_rotation);
}

vec3 ik_bone::get_head() const
{
	if (m_parent)
		return vec3(m_parent->get_model() * vec4(1, 0, 0, 1));
	return vec3{ 0.0f };
}

vec3 ik_bone::get_tail() const
{
	return vec3(get_model() * vec4(1, 0, 0, 1));
}

void ik_bone::set_tail(const vec3 & t)
{
	vec3 prev = m_parent ? glm::normalize(m_parent->get_tail()-m_parent->get_head()) : vec3{1, 0, 0};
	vec3 cur = glm::normalize(t - get_head());
	m_rotation = glm::normalize(quat{ prev,cur });
}


void ik_chain::run_solver()
{
	switch (m_solver)
	{
	case ik_chain::e_2BoneIK:
		if (m_bones.size() == 2)
		{
			m_end_effector.z = 0.0f;
			m_status = run_2_bone_ik();
		}
		else
		{
			ImGui::TextColored(ImColor(255, 0, 0), "Bone size must be 2");
			m_status = e_OutofReach;
		}
		break;
	case ik_chain::e_CCD:
		for (int i = 0; i < m_iterations; i++)
			if ((m_status = run_ccd()) != e_Running)
				break;
		break;
	case ik_chain::e_FABRIK:
		for (int i = 0; i < m_iterations; i++)
			if ((m_status = run_FABRIK()) != e_Running)
				break;
		break;
	default:
		break;
	}

	switch (m_status)
	{
	case ik_chain::e_Running:
		ImGui::TextColored(ImColor(255, 255, 255), "Running");
		break;
	case ik_chain::e_Finished:
		ImGui::TextColored(ImColor(55, 255, 55), "Finished");
		break;
	case ik_chain::e_OutofReach:
		ImGui::TextColored(ImColor(255, 55, 55), "Failed");
		break;
	}
}

bool ik_chain::is_end_outofreach()
{
	float length = std::accumulate(m_bones.begin(), m_bones.end(), 0.0f,
		[](float acc, const ik_bone* b)->float
	{ return acc + b->m_length; });
	if (glm::length(m_end_effector) > length)
		return true;
	if (m_bones.size() == 2)
	{
		float diff = fabsf(m_bones[0]->m_length - m_bones[1]->m_length);
		if (glm::length(m_end_effector) < diff)
			return true;
	}
	return false;
}

ik_chain::ik_chain(transform3d tr, size_t start_count)
	: scene_object("joint", tr, nullptr, nullptr)
{
	ik_bone * last{nullptr};
	for (size_t i = 0; i < start_count; i++)
	{

		ik_bone* b = new ik_bone(last);
		m_bones.push_back(b);
		if (last == nullptr)
			m_root = b;
		last = b;
	}
	m_end_effector = vec3{ (float)start_count,0,0 };
}
void ik_chain::draw(Shader_Program * shader)
{
	for(auto& b : m_bones)
	{
		shader->set_uniform("M", m_transform.get_model() * b->get_model());
		if (shader == renderer->color_shader)
		{
			shader->set_uniform("color", renderer->compute_selection_color());
			m_model->draw(shader, nullptr,false);
		}
		else
			m_model->draw(shader, nullptr);
	}

	shader->set_uniform("M", glm::scale(glm::translate(m_transform.get_model(),m_end_effector), vec3(0.5f)));
	if (shader == renderer->color_shader)
	{
		shader->set_uniform("color", renderer->compute_selection_color());
		renderer->get_model("octohedron")->draw(shader, nullptr, false);
	}
	else
		renderer->get_model("octohedron")->draw(shader, nullptr);
}
void ik_chain::draw_GUI()
{
	ImGui::Text("IK Chain");
	if (m_selected < m_bones.size())
	{
		ImGui::Text(("Current Bone: " + std::to_string(m_selected)).c_str());
		ImGui::DragFloat("Bone length", &m_bones[m_selected]->m_length, 0.1f, 0.1f, 10.0f);
		ImGui::InputFloat4("Bone rotation", &m_bones[m_selected]->m_rotation.x);
	}
	else
	{
		ImGui::Text("Selected End Effector");
	}

	ImGui::NewLine();
	ImGui::NewLine();

	ImGui::Text("Solver:");
	const char * modes[]{ "2-Bone IK", "CCD", "FABRIK" };
	if (ImGui::BeginCombo("Solver", modes[m_solver]))
	{
		for (int n = 0; n < 3; n++)
		{
			if (ImGui::Selectable(modes[n], n==m_solver))
				m_solver = static_cast<e_Solver>(n);
		}
		ImGui::EndCombo();
	}
	ImGui::Checkbox("Active", &m_active);
	if (m_active) run_solver();
}
ik_chain::e_Status ik_chain::run_2_bone_ik()
{
	if (glm::length(m_bones.back()->get_tail() - m_end_effector) < m_epsilon)
		return e_Finished;
	if (is_end_outofreach())
		return e_OutofReach;


	float x = m_end_effector.x;
	float y = m_end_effector.y;
	float d1 = m_bones[0]->m_length;
	float d2 = m_bones[1]->m_length;
	float theta_2 = acosf((x*x + y*y - ((d1*d1) + (d2*d2))) / (2.0f*d1*d2));
	float theta_1 = atan2f(
		y*(d1 + d2 * cosf(theta_2)) - x * (d2*sinf(theta_2)),
		x*(d1 + d2 * cosf(theta_2)) + y * (d2*sinf(theta_2)) );
	m_bones[0]->m_rotation = normalize(quat{ vec3{0.0f, 0.0f, theta_1} });
	m_bones[1]->m_rotation = normalize(quat{ vec3{0.0f, 0.0f, theta_2} });
	return e_Running;
}
ik_chain::e_Status ik_chain::run_ccd()
{
	if (glm::length(m_bones.back()->get_tail() - m_end_effector) < m_epsilon)
		return e_Finished;
	if(is_end_outofreach())
		return e_OutofReach;


	for (int i = m_bones.size() - 1; i >= 0; i--)
	{
		vec3 head = m_bones[i]->get_head();
		vec3 chain_tail = m_bones.back()->get_tail();

		vec3 to_tail = normalize(chain_tail - head);
		vec3 to_end_effector = normalize(m_end_effector - head);

		if (1.0f - glm::dot(to_tail, to_end_effector) < glm::epsilon<float>())
			continue;

		quat q{ to_tail, to_end_effector };
		m_bones[i]->m_rotation = normalize(m_bones[i]->m_rotation * normalize(q));
	}
	return e_Running;
}

ik_chain::e_Status ik_chain::run_FABRIK()
{
	if (glm::length(m_bones.back()->get_tail() - m_end_effector) < m_epsilon)
		return e_Finished;
	if (is_end_outofreach())
		return e_OutofReach;

	std::vector<vec3> m_positions;
	for (size_t i = 0; i < m_bones.size(); i++)
		m_positions.push_back(m_bones[i]->get_head());
	m_positions.push_back(m_end_effector);
	for (int i = (int)m_bones.size() - 1; i >= 0; i--)
		m_positions[i] = m_positions[i + 1] - m_bones[i]->m_length * glm::normalize(m_positions[i + 1] - m_positions[i]);
	m_positions.front() = vec3(0);
	for (int i = 1; i <= (int)m_bones.size(); i++)
		m_positions[i] = m_positions[i - 1] - m_bones[i-1]->m_length * glm::normalize(m_positions[i - 1] - m_positions[i]);
	for (int i = 0; i < (int)m_bones.size(); i++)
		m_bones[i]->set_tail(m_positions[i + 1]);
	return e_Running;
}
