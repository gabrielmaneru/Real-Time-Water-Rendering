/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	scene.h
Purpose: Scene Manager
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#include "scene.h"
#include <fstream>
#include <sstream>
#include <imgui/imgui.h>
#include <utils/math_utils.h>
#include <graphics/renderer.h>
#include <GLFW/glfw3.h>
#include <platform/window_manager.h>
c_scene * scene = new c_scene;
bool c_scene::load_scene(std::string path)
{
	std::string real_path = "./data/scenes/" + path + ".json";
	std::ifstream file;
	file.open(real_path);
	if (file.is_open())
	{
		std::string stream;
		file.seekg(0, std::ios::end);
		stream.reserve(file.tellg());
		file.seekg(0, std::ios::beg);
		stream.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
		
		{
			// Read File
			size_t it{0u};
			bool is_char{ false };
			int lvl_bra{ 0 };
			int lvl_sqbra{ 0 };
			while (it < stream.size())
			{
				if (is_char && (stream[it] != '"'))
				{
					it++;
				}
				else
				{
					switch (stream[it])
					{
					case '{':
						lvl_bra++;
						it++;
						break;
					case '}':
						lvl_bra--;
						it++;
						if (lvl_bra == 1 && lvl_sqbra == 1)
						{
							stream.insert(it, 1, 0x1D);
							it++;
						}
						break;
					case '[':
						lvl_sqbra++;
						it++;
						break;
					case ']':
						lvl_sqbra--;
						it++;
						break;
					case '"':
						is_char = !is_char;
						stream.erase(it, 1);
						break;
					case ':':
					case ',':
					case '.':
					case '-':
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						it++;
						break;
					default:
						stream.erase(it, 1);
						break;
					}
				}
			}

			// Invalid File
			if (lvl_bra != 0 || lvl_sqbra != 0 || is_char)
			{
				file.close();
				return false;
			}


			// Create Objects
			{
				size_t obj_location = stream.find("objects");
				std::string objs = stream.substr(obj_location);
				objs = objs.substr(objs.find_first_of('[') + 1, objs.find_first_of(']') - objs.find_first_of('['));
				while (!objs.empty())
				{
					std::string obj = objs.substr(0, objs.find_first_of(0x1D));
					objs = objs.substr(objs.find_first_of(0x1D)+2);

					// Get Mesh
					std::string mesh_name = obj.substr(obj.find("mesh") + 5, obj.find_first_of(',') - obj.find("mesh") - 5);

					// Get Transform
					vec3 pos{-1.0};
					obj = obj.substr(obj.find("translation"));
					pos.x = (float)std::atof(obj.substr(obj.find_first_of('x') + 2, obj.find_first_of('y') - obj.find_first_of('x') - 3).c_str());
					pos.y = (float)std::atof(obj.substr(obj.find_first_of('y')+2, obj.find_first_of('z') - obj.find_first_of('y')-3).c_str());
					pos.z = (float)std::atof(obj.substr(obj.find_first_of('z')+2, obj.find_first_of('}') - obj.find_first_of('z')-2).c_str());
					vec3 rot{ -1.0 };
					obj = obj.substr(obj.find("rotate"));
					rot.x = (float)std::atof(obj.substr(obj.find_first_of('x') + 2, obj.find_first_of('y') - obj.find_first_of('x') - 3).c_str());
					rot.y = (float)std::atof(obj.substr(obj.find_first_of('y') + 2, obj.find_first_of('z') - obj.find_first_of('y') - 3).c_str());
					rot.z = (float)std::atof(obj.substr(obj.find_first_of('z') + 2, obj.find_first_of('}') - obj.find_first_of('z') - 2).c_str());
					float scl{ -1.0 };
					obj = obj.substr(obj.find("scale"));
					scl = (float)std::atof(obj.substr(obj.find_first_of('x') + 2, obj.find_first_of('y') - obj.find_first_of('x') - 3).c_str());
					transform3d tr;
					tr.set_tr(pos, scl, normalize(quat(glm::radians(rot))));
					
					// Get Animator
					animator* anim{ nullptr };
					auto t = obj.find("animator");
					if (t < obj.size())
					{
						obj = obj.substr(t);
						anim = new animator;
						std::string s = obj.substr(obj.find("active") + 7, obj.find("playback") - obj.find("active") - 8);
						anim->m_active = (bool)std::atoi(s.c_str());
						s = obj.substr(obj.find("playback") + 9, obj.find("num_anim") - obj.find("playback") - 10);
						anim->m_playback = (bool)std::atoi(s.c_str());
						s = obj.substr(obj.find("num_anim") + 9, obj.find("speed") - obj.find("num_anim") - 10);
						anim->m_current_animation = std::atoi(s.c_str());
						s = obj.substr(obj.find("speed") + 6, obj.find("}") - obj.find("speed") - 6);
						anim->m_speed = (double)std::atof(s.c_str());
					}

					curve_interpolator * m_curve{ nullptr };
					t = obj.find("curve");
					if (t < obj.size())
					{
						obj = obj.substr(t);
						m_curve = new curve_interpolator;
						std::string s = obj.substr(obj.find(":")+1, obj.find("}") - obj.find(":")-1);
						if (s == "line")
							m_curve->m_curve = renderer->m_curve_line;
						if (s == "hermite")
							m_curve->m_curve = renderer->m_curve_hermite;
						if (s == "catmull")
							m_curve->m_curve = renderer->m_curve_catmull;
						if (s == "bezier")
							m_curve->m_curve = renderer->m_curve_bezier;
					}

					m_objects.push_back(new scene_object(mesh_name, tr, anim, m_curve));
				}
			}
		}

		file.close();
		return true;
	}
	return false;
}

bool c_scene::init()
{
	if (!load_scene(m_scene_name))
		return false;

	transform3d tr;
	tr.set_scl(.5f);
	{
		light_data ld;
		ld.m_att_factor = { 0.f,0.001f,0.001f };
		tr.set_pos({ 5,22,7 });
		m_lights.push_back(new light(tr, ld));
		tr.set_pos({ 3.5f,65.0f,-99.0f });
		ld.m_diffuse = { 1,0,1 };
		m_lights.push_back(new light(tr, ld));
	}
	light_data ld;
	for (int i = 0; i < m_num_lights/2; i++)
	{
		tr.set_pos({ random_float(17.5f, 32.5f), random_float(0.f, 50.f),random_float(-106.f, 94.f) });
		ld.m_diffuse = { random_float(0.3f, 1.f), random_float(0.3f, 1.f),random_float(0.3f, 1.f) };
		ld.m_specular = ld.m_diffuse;
		m_lights.push_back(new light(tr, ld));
	}

	for (int i = 0; i < m_num_lights/2; i++)
	{
		tr.set_pos({ random_float(-27.5f, -12.5f), random_float(0.f, 50.f),random_float(-106.f, 94.f) });
		ld.m_diffuse = { random_float(0.3f, 1.f), random_float(0.3f, 1.f),random_float(0.3f, 1.f) };
		ld.m_specular = ld.m_diffuse;
		m_lights.push_back(new light(tr, ld));
	}
	for (auto& l : m_lights)
		l->time = random_float(0.0f, glm::pi<float>());
	return true;
}

void c_scene::update()
{
	if (m_animated_scene)
	{
		for (auto& l : m_lights)
		{
			vec3 pos = l->m_transform.get_pos();
			pos.y += 0.5f*sin(l->time);
			l->time += 1/60.f;
			pos.z += 1.0f;
			if (pos.z > 94.f)
				pos.z -= 200.f;
			l->m_transform.set_pos(pos);
		}
	}
}

void c_scene::draw_objs(Shader_Program * shader)
{
	for (auto p_obj : m_objects)
		p_obj->draw(shader);
}

void c_scene::draw_lights(Shader_Program * shader)
{
	shader->set_uniform("la", light_data::m_ambient);
	for (auto p_li : m_lights)
		p_li->draw(shader);
}

void c_scene::draw_debug_lights(Shader_Program * shader)
{
	transform3d tr;
	tr.set_scl(.5f);

	for (auto p_li : m_lights)
	{
		tr.set_pos(p_li->m_transform.get_pos());
		shader->set_uniform("M", tr.get_model());
		shader->set_uniform("selection_color", renderer->compute_selection_color());
		renderer->get_model("sphere")->draw(shader, nullptr);
	}
}

void c_scene::shutdown()
{
	for (auto p_obj : m_objects)
		delete p_obj;
	m_objects.clear();
	for (auto p_li : m_lights)
		delete p_li;
	m_lights.clear();
}

void c_scene::drawGUI()
{
	ImGui::Text("# Objects #");
	if (window_manager->is_key_down(GLFW_KEY_LEFT_CONTROL)
	&&  window_manager->is_key_triggered(GLFW_KEY_R))
	{
		shutdown();
		init();
	}
	if (ImGui::TreeNode("Scene Options"))
	{
		if (ImGui::Button("Reload"))
		{
			shutdown();
			init();
		}
		ImGui::Checkbox("Animate Scene", &m_animated_scene);
		ImGui::InputInt("Num Lights", &m_num_lights);
		if ((m_lights.size() > 1) && ImGui::TreeNode("Change ALL Light Values"))
		{
			vec3 m_diffuse = m_lights[1]->m_ldata.m_diffuse;
			vec3 m_specular=m_lights[1]->m_ldata.m_specular;
			vec3 m_att_factor = m_lights[1]->m_ldata.m_att_factor;
			if (ImGui::InputFloat3("ALL Diffuse", &m_diffuse.x))
				for (auto l : m_lights)
					l->m_ldata.m_diffuse = m_diffuse;
			if (ImGui::InputFloat3("ALL Specular", &m_specular.x))
				for (auto l : m_lights)
					l->m_ldata.m_specular = m_specular;
			if (ImGui::InputFloat3("ALL AttFactor", &m_att_factor.x))
				for (auto l : m_lights)
					l->m_ldata.m_att_factor = m_att_factor;


			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Objects List"))
	{
		if (ImGui::Button("Create"))
			m_objects.push_back(new scene_object{ "cube" , {}, nullptr, nullptr });
		for (int i = 0; i < m_objects.size(); i++)
		{
			ImGui::PushID(i);
			std::string tree_name = m_objects[i]->m_model
				? m_objects[i]->m_model->m_name
				: "Unknown";

			if (ImGui::TreeNode(tree_name.c_str()))
			{
				const char* current = m_objects[i]->m_model ? m_objects[i]->m_model->m_name.c_str() : "Unknown";
				if (ImGui::BeginCombo("Mesh", current))
				{
					for (size_t n = 0; n < renderer->m_models.size(); n++)
					{
						bool is_selected = (m_objects[i]->m_model == renderer->m_models[n]);
						if (ImGui::Selectable(renderer->m_models[n]->m_name.c_str(), is_selected))
							m_objects[i]->m_model = renderer->m_models[n];
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
					
				}
				bool chng{ false };
				if (ImGui::DragFloat3("Position", &m_objects[i]->m_transform.m_tr.m_pos.x, .1f))chng = true;
				vec3 eu_angles = degrees(glm::eulerAngles(m_objects[i]->m_transform.m_tr.m_rot));
				if (ImGui::DragFloat3("Rotation", &eu_angles.x))
				{
					m_objects[i]->m_transform.m_tr.m_rot = normalize(quat(radians(eu_angles)));
					chng = true;
				}
				if (ImGui::DragFloat("Scale", &m_objects[i]->m_transform.m_tr.m_scl, .1f, .001f, 99999999.f))chng = true;
				if (chng)m_objects[i]->m_transform.m_tr.upd();
				if (m_objects[i]->m_animator && ImGui::TreeNode("Animator"))
					m_objects[i]->m_animator->draw_GUI(), ImGui::TreePop();
				if (m_objects[i]->m_curve && ImGui::TreeNode("Curve"))
					m_objects[i]->m_curve->draw_GUI(), ImGui::TreePop();
				if (ImGui::Button("Delete"))
				{
					delete m_objects[i];
					m_objects.erase(m_objects.begin()+i);
					i--;
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	}


	ImGui::NewLine();
	ImGui::Text("# Lights #");
	if (ImGui::TreeNode("Global Options"))
	{
		ImGui::PushID(5);
		static bool display_break{ false };
		ImGui::Checkbox("", &display_break);
		ImGui::SameLine();
		if (display_break)
			ImGui::DragFloat3("Ambient", &light_data::m_ambient.x, 0.01f, 0.0f, 1.0f);
		else if (ImGui::DragFloat("Ambient", &light_data::m_ambient.x, 0.01f, 0.0f, 1.0f))
			light_data::m_ambient.y = light_data::m_ambient.z = light_data::m_ambient.x;

		ImGui::SliderFloat("AttMax", &light_data::m_att_max, 0.001f, 1.0f);
		ImGui::PopID();
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Lights List"))
	{
		if (ImGui::Button("Create"))
			m_lights.push_back(new light);
		for (int i = 0; i < m_lights.size(); i++)
		{
			ImGui::PushID(i);

			if (ImGui::TreeNode("Light"))
			{
				if (ImGui::DragFloat3("Position", &scene->m_lights[i]->m_transform.m_tr.m_pos.x, .1f))
					scene->m_lights[i]->m_transform.m_tr.upd();
				scene->m_lights[i]->m_ldata.drawGUI();

				if (ImGui::Button("Delete"))
				{
					delete m_lights[i];
					m_lights.erase(m_lights.begin() + i);
					i--;
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	}
}
