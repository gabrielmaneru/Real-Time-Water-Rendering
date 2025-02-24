/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	scene.h
Purpose: Scene Manager
Author: Gabriel Ma�eru - gabriel.m
- End Header --------------------------------------------------------*/

#include "scene.h"
#include <fstream>
#include <sstream>
#include <imgui/imgui.h>
#include <utils/math_utils.h>
#include <graphics/renderer.h>
#include <GLFW/glfw3.h>
#include <platform/window_manager.h>
#include <platform/editor.h>
c_scene * scene = new c_scene;
const char end_of_item{ 0x1D };
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
							stream.insert(it, 1, end_of_item);
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
				stream = stream.substr(stream.find("objects"));
				std::string objs = stream.substr(stream.find_first_of('[') + 1, stream.find_first_of(']') - stream.find_first_of('['));
				while (!objs.empty())
				{
					std::string obj = objs.substr(0, objs.find_first_of(end_of_item));
					objs = objs.substr(objs.find_first_of(end_of_item) + 2);

					// Get Mesh
					std::string mesh_name = obj.substr(obj.find("mesh") + 5, obj.find_first_of(',') - obj.find("mesh") - 5);

					// Get Transform
					vec3 pos{ -1.0 };
					obj = obj.substr(obj.find("translation"));
					pos.x = (float)std::atof(obj.substr(obj.find_first_of('x') + 2, obj.find_first_of('y') - obj.find_first_of('x') - 3).c_str());
					pos.y = (float)std::atof(obj.substr(obj.find_first_of('y') + 2, obj.find_first_of('z') - obj.find_first_of('y') - 3).c_str());
					pos.z = (float)std::atof(obj.substr(obj.find_first_of('z') + 2, obj.find_first_of('}') - obj.find_first_of('z') - 2).c_str());
					vec3 rot{ -1.0 };
					obj = obj.substr(obj.find("rotate"));
					rot.x = (float)std::atof(obj.substr(obj.find_first_of('x') + 2, obj.find_first_of('y') - obj.find_first_of('x') - 3).c_str());
					rot.y = (float)std::atof(obj.substr(obj.find_first_of('y') + 2, obj.find_first_of('z') - obj.find_first_of('y') - 3).c_str());
					rot.z = (float)std::atof(obj.substr(obj.find_first_of('z') + 2, obj.find_first_of('}') - obj.find_first_of('z') - 2).c_str());
					vec3 scl{ -1.0 };
					obj = obj.substr(obj.find("scale"));
					scl.x = (float)std::atof(obj.substr(obj.find_first_of('x') + 2, obj.find_first_of('y') - obj.find_first_of('x') - 3).c_str());
					scl.y = (float)std::atof(obj.substr(obj.find_first_of('y') + 2, obj.find_first_of('z') - obj.find_first_of('y') - 3).c_str());
					scl.z = (float)std::atof(obj.substr(obj.find_first_of('z') + 2, obj.find_first_of('}') - obj.find_first_of('z') - 2).c_str());
					transform3d tr;
					tr.set_tr(pos, scl, normalize(quat(glm::radians(rot))));

					// Get Animator
					animator* anim{ nullptr };
					auto t = obj.find("animator");
					if (t < obj.size())
					{
						obj = obj.substr(t);
						anim = new animator;

						std::string s;
						s = obj.substr(obj.find("num_anim") + 9, obj.find("active") - obj.find("num_anim") - 10);
						anim->m_current_animation = std::atoi(s.c_str());
						s = obj.substr(obj.find("active") + 7, obj.find("playback") - obj.find("active") - 8);
						anim->m_active = (bool)std::atoi(s.c_str());
						s = obj.substr(obj.find("playback") + 9, obj.find("speed") - obj.find("playback") - 10);
						anim->m_playback = (bool)std::atoi(s.c_str());
						s = obj.substr(obj.find("speed") + 6, obj.find("}") - obj.find("speed") - 6);
						anim->m_max_speed = (double)std::atof(s.c_str());
					}

					curve_interpolator * m_curve{ nullptr };
					t = obj.find("curve_interpolator");
					if (t < obj.size())
					{
						obj = obj.substr(t);
						m_curve = new curve_interpolator;

						std::string s;
						s = obj.substr(obj.find("curve:") + 6, obj.find("active") - obj.find("curve:") - 7);
						for (auto p_c : renderer->m_curves)
						{
							if (p_c->m_name == s)
							{
								m_curve->m_actual_curve = p_c;
								break;
							}
						}
						if (m_curve->m_actual_curve == nullptr)
						{
							delete m_curve;
							m_curve = nullptr;
							break;
						}

						s = obj.substr(obj.find("active") + 7, obj.find("playback") - obj.find("active") - 8);
						m_curve->m_active = (bool)std::atoi(s.c_str());
						s = obj.substr(obj.find("playback") + 9, obj.find("speed") - obj.find("playback") - 10);
						m_curve->m_playback = (bool)std::atoi(s.c_str());
						s = obj.substr(obj.find("speed") + 6, obj.find("}") - obj.find("speed") - 6);
						m_curve->m_max_speed = (double)std::atof(s.c_str());
					}

					m_objects.push_back(new scene_object(mesh_name, tr, anim, m_curve));
				}
			}

			// Create Decals
			auto d = stream.find("decals");
			if(d < stream.size())
			{

				stream = stream.substr();
				std::string dcls = stream.substr(stream.find_first_of('[') + 1, stream.find_first_of(']') - stream.find_first_of('['));
				while (!dcls.empty())
				{
					std::string dcl = dcls.substr(0, dcls.find_first_of(end_of_item));
					dcls = dcls.substr(dcls.find_first_of(end_of_item) + 2);

					// Get Texture
					dcl = dcl.substr(dcl.find("diffuse"));
					std::string diffuse_txt = dcl.substr(dcl.find("diffuse") + 8, dcl.find_first_of(',') - 8);
					std::string normal_txt = dcl.substr(dcl.find("normal") + 7, dcl.find_first_of(',') - 8);

					// Get Transform
					vec3 pos{ -1.0 };
					dcl = dcl.substr(dcl.find("translation"));
					pos.x = (float)std::atof(dcl.substr(dcl.find_first_of('x') + 2, dcl.find_first_of('y') - dcl.find_first_of('x') - 3).c_str());
					pos.y = (float)std::atof(dcl.substr(dcl.find_first_of('y') + 2, dcl.find_first_of('z') - dcl.find_first_of('y') - 3).c_str());
					pos.z = (float)std::atof(dcl.substr(dcl.find_first_of('z') + 2, dcl.find_first_of('}') - dcl.find_first_of('z') - 2).c_str());
					vec3 rot{ -1.0 };
					dcl = dcl.substr(dcl.find("rotate"));
					rot.x = (float)std::atof(dcl.substr(dcl.find_first_of('x') + 2, dcl.find_first_of('y') - dcl.find_first_of('x') - 3).c_str());
					rot.y = (float)std::atof(dcl.substr(dcl.find_first_of('y') + 2, dcl.find_first_of('z') - dcl.find_first_of('y') - 3).c_str());
					rot.z = (float)std::atof(dcl.substr(dcl.find_first_of('z') + 2, dcl.find_first_of('}') - dcl.find_first_of('z') - 2).c_str());
					vec3 scl{ -1.0 };
					dcl = dcl.substr(dcl.find("scale"));
					scl.x = (float)std::atof(dcl.substr(dcl.find_first_of('x') + 2, dcl.find_first_of('y') - dcl.find_first_of('x') - 3).c_str());
					scl.y = (float)std::atof(dcl.substr(dcl.find_first_of('y') + 2, dcl.find_first_of('z') - dcl.find_first_of('y') - 3).c_str());
					scl.z = (float)std::atof(dcl.substr(dcl.find_first_of('z') + 2, dcl.find_first_of('}') - dcl.find_first_of('z') - 2).c_str());

					transform3d tr;
					tr.m_tr.m_model = glm::translate(mat4(1.0f), pos);
					tr.m_tr.m_model = glm::rotate(tr.m_tr.m_model, glm::radians(rot.x), vec3(1, 0, 0));
					tr.m_tr.m_model = glm::rotate(tr.m_tr.m_model, glm::radians(rot.y), vec3(0, 1, 0));
					tr.m_tr.m_model = glm::rotate(tr.m_tr.m_model, glm::radians(rot.z), vec3(0, 0, 1));
					tr.m_tr.m_model = glm::scale(tr.m_tr.m_model, scl);
					tr.m_tr.should_update = false;
					m_decals.push_back(new decal(diffuse_txt, normal_txt, tr));
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
	tr.set_pos({ 250,1250,3000 });
	tr.set_scl(vec3(1.f));

	light_data ld;
	m_dir_light = new dir_light(tr, ld);

	return true;
}

void c_scene::update()
{
	for (auto p_obj : m_objects)
		p_obj->update();
}

void c_scene::draw_objs(Shader_Program * shader)
{
	for (auto p_obj : m_objects)
			p_obj->draw(shader);
}

void c_scene::draw_point_lights(Shader_Program * shader)
{
	shader->set_uniform("la", light_data::m_ambient);
	for (auto p_li : m_point_lights)
		p_li->draw(shader);
}

void c_scene::draw_decals(Shader_Program * shader)
{
	for (auto p_d : m_decals)
		p_d->draw(shader);
}

void c_scene::draw_debug_lights(Shader_Program * shader)
{
	transform3d tr;
	tr.set_scl(vec3(.5f));

	for (auto p_li : m_point_lights)
	{
		tr.set_pos(p_li->m_transform.get_pos());
		shader->set_uniform("M", tr.get_model());
		shader->set_uniform("selection_color", renderer->compute_selection_color());
		renderer->get_model("sphere")->draw(shader, nullptr, false);
	}
	if(m_dir_light)
	{
		tr.set_pos(m_dir_light->m_transform.get_pos());
		shader->set_uniform("M", tr.get_model());
		renderer->get_model("octohedron")->draw(shader, nullptr, false);
	}
}

void c_scene::draw_debug_curves(Shader_Program * shader)
{
	transform3d tr;
	for (auto p_obj : m_objects)
	{
		if (p_obj->m_curve_interpolator == nullptr || p_obj->m_curve_interpolator->m_actual_curve == nullptr)
			continue;

		const curve_base* curve = p_obj->m_curve_interpolator->m_actual_curve;
		const size_t evals = 1000;
		float dur = curve->duration();
		float step = dur / (float)evals;
		
		for (size_t i = 0; i < evals; i++)
		{
			float t_0 = step * (float)i;
			float t_1 = step * (float)(i+1);
		
			vec3 pos_0 = p_obj->m_transform.get_pos() + curve->evaluate(t_0);
			vec3 pos_1 = p_obj->m_transform.get_pos() + curve->evaluate(t_1);
		
			mat4 model = glm::translate(mat4(1.0f), lerp(pos_0, pos_1, 0.5f));
			model = glm::scale(model, abs(pos_1 - pos_0) + vec3(0.1f));
			shader->set_uniform("M", model);
		
			renderer->get_model("sphere")->draw(shader, nullptr);
		}
		
		
		for (size_t i = 0; i < curve->m_frames.size(); i += curve->point_stride)
		{
			vec3 P0 = p_obj->m_transform.get_pos() + curve->m_frames[i].first;
			mat4 model = glm::translate(mat4(1.0f), P0);
			model = glm::scale(model, vec3(0.5f, 0.3f, 0.5f));
			shader->set_uniform("M", model);
			renderer->set_debug_color({ 0,0,1 });
			renderer->get_model("sphere")->draw(shader, nullptr);
			renderer->reset_debug_color();
		}

		for (auto& k : curve->m_length_table)
		{
			vec3 pos = p_obj->m_transform.get_pos() + curve->evaluate(k.m_param_value * dur);

			mat4 model = glm::translate(mat4(1.0f), pos);
			model = glm::scale(model, vec3(0.3f, 0.5f, 0.3f));
			shader->set_uniform("M", model);
			renderer->set_debug_color({ 1,0,0 });
			renderer->get_model("sphere")->draw(shader, nullptr);
			renderer->reset_debug_color();
		}
	}
}

void c_scene::shutdown()
{
	for (auto p_obj : m_objects)
		delete p_obj;
	m_objects.clear();
	for (auto p_li : m_point_lights)
		delete p_li;
	m_point_lights.clear();
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

			bool is_selected = m_objects[i] == editor->m_selected;
			if (ImGui::Selectable(tree_name.c_str(), is_selected))
				editor->m_selected = m_objects[i];
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
			m_point_lights.push_back(new point_light);
		for (int i = 0; i < m_point_lights.size(); i++)
		{
			ImGui::PushID(i);
			bool is_selected = m_point_lights[i] == editor->m_selected;
			if (ImGui::Selectable("Light", is_selected))
				editor->m_selected = m_objects[i];
			ImGui::PopID();
		}

		if (m_dir_light)
		{
			ImGui::NewLine();
			bool is_selected = m_dir_light == editor->m_selected;
			if (ImGui::Selectable("Directional Light", is_selected))
				editor->m_selected = m_dir_light;
		}
		ImGui::TreePop();
	}
}
