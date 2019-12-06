#include "ik_bone.h"
#include <graphics/renderer.h>
#include <imgui/imgui.h>
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
		else
			last->m_children.push_back(b);
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
				m_solver = n;
		}
		ImGui::EndCombo();
	}
	ImGui::Checkbox("Active", &m_active);
	switch (m_solver)
	{
	case 0:
		if (m_active)
		{
			if (m_bones.size() == 2)
			{
				m_end_effector.z = 0.0f;
				run_2_bone_ik();
			}
			else
				ImGui::TextColored(ImColor(255, 0, 0), "Bone size must be 2");
		}
		break;

	case 1:
		if (m_active)
		{
			bool finished{ false };
			for (int i = 0; i < m_iterations; i++)
				if (finished = run_ccd())
					break;

			if (finished)
				ImGui::TextColored(ImColor(0, 255, 0), "Finished");
			else
				ImGui::TextColored(ImColor(255, 0, 0), "Fail");
		}
		break;
	default:
		break;
	}

}
void ik_chain::run_2_bone_ik()
{
	float x = m_end_effector.x;
	float y = m_end_effector.y;
	float dist = glm::length(vec2(x, y));
	float d1 = m_bones[0]->m_length;
	float d2 = m_bones[1]->m_length;

	if (dist < d1 - d2 || dist > d1 + d2)
		return;

	float theta_2 = acosf((x*x + y*y - ((d1*d1) + (d2*d2))) / (2.0f*d1*d2));
	float theta_1 = atan2f(
		y*(d1 + d2 * cosf(theta_2)) - x * (d2*sinf(theta_2)),
		x*(d1 + d2 * cosf(theta_2)) + y * (d2*sinf(theta_2)) );

	m_bones[0]->m_rotation = normalize(quat{ vec3{0.0f, 0.0f, theta_1} });
	m_bones[1]->m_rotation = normalize(quat{ vec3{0.0f, 0.0f, theta_2} });
}
bool ik_chain::run_ccd()
{
	if (glm::length(m_bones.back()->get_tail() - m_end_effector) < glm::epsilon<float>())
		return true;
	else
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
	return false;
}