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
						anim->m_speed = (double)std::atof(s.c_str());
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
						m_curve->m_speed = (double)std::atof(s.c_str());
					}

					m_objects.push_back(new scene_object(mesh_name, tr, anim, m_curve));
				}
			}

			// Create Decals
			/*{
				stream = stream.substr(stream.find("decals"));
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
					tr.set_tr(pos, scl, normalize(quat(glm::radians(rot))));
					
					m_decals.push_back(new decal(diffuse_txt, normal_txt, tr));
				}
			}*/
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
	tr.set_scl(vec3(.5f));
	{
		light_data ld;
		ld.m_att_factor = { 0.f,0.001f,0.001f };
		tr.set_pos({ 7,22,5 });
		m_lights.push_back(new light(tr, ld));
		tr.set_pos({ -99.0f,65.0f,3.5f });
		ld.m_diffuse = { 1,0,1 };
		m_lights.push_back(new light(tr, ld));
	}
	light_data ld;
	for (int i = 0; i < m_num_lights/2; i++)
	{
		tr.set_pos({ random_float(-106.f, 94.f), random_float(0.f, 50.f),random_float(17.5f, 32.5f) });
		ld.m_diffuse = { random_float(0.3f, 1.f), random_float(0.3f, 1.f),random_float(0.3f, 1.f) };
		m_lights.push_back(new light(tr, ld));
	}

	for (int i = 0; i < m_num_lights/2; i++)
	{
		tr.set_pos({ random_float(-106.f, 94.f), random_float(0.f, 50.f),random_float(-27.5f, -12.5f) });
		ld.m_diffuse = { random_float(0.3f, 1.f), random_float(0.3f, 1.f),random_float(0.3f, 1.f) };
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
			pos.x += 1.0f;
			if (pos.x > 94.f)
				pos.x -= 200.f;
			l->m_transform.set_pos(pos);
		}
	}
	for (auto p_obj : m_objects)
		p_obj->update();
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
	tr.set_scl(vec3(.5f));

	for (auto p_li : m_lights)
	{
		tr.set_pos(p_li->m_transform.get_pos());
		shader->set_uniform("M", tr.get_model());
		shader->set_uniform("selection_color", renderer->compute_selection_color());
		renderer->get_model("sphere")->draw(shader, nullptr);
	}
}

void c_scene::draw_debug_curves(Shader_Program * shader)
{
	transform3d tr;
	for (auto p_obj : m_objects)
	{
		if (p_obj->m_curve_interpolator == nullptr || p_obj->m_curve_interpolator->m_actual_curve == nullptr)
			continue;

		const size_t evals = 1000;
		float dur = p_obj->m_curve_interpolator->m_actual_curve->duration();
		float step = dur / (float)evals;

		for (size_t i = 0; i < evals; i++)
		{
			float t_0 = step * (float)i;
			float t_1 = step * (float)(i+1);

			vec3 pos_0 = p_obj->m_transform.get_pos() + p_obj->m_curve_interpolator->m_actual_curve->evaluate(t_0);
			vec3 pos_1 = p_obj->m_transform.get_pos() + p_obj->m_curve_interpolator->m_actual_curve->evaluate(t_1);

			mat4 model = glm::translate(mat4(1.0f), lerp(pos_0, pos_1, 0.5f));
			model = glm::scale(model, abs(pos_1 - pos_0) + vec3(0.1f));
			shader->set_uniform("M", model);

			renderer->get_model("cube")->draw(shader, nullptr);
		}

		const curve_base* curve= p_obj->m_curve_interpolator->m_actual_curve;
		if (dynamic_cast<const curve_line*>(curve) != nullptr
		||  dynamic_cast<const curve_catmull*>(curve) != nullptr)
		{
			for (auto p : curve->m_frames)
			{
				mat4 model = glm::translate(mat4(1.0f), p_obj->m_transform.get_pos() + p.first);
				model = glm::scale(model, vec3(0.5f));
				shader->set_uniform("M", model);
				renderer->get_model("sphere")->draw(shader, nullptr);
			}
		}
		else
		if (dynamic_cast<const curve_hermite*>(curve) != nullptr
		||  dynamic_cast<const curve_bezier*>(curve) != nullptr)
		{
			for (size_t i = 0; i < curve->m_frames.size() - 3; i += 3)
			{
				vec3 P0 = p_obj->m_transform.get_pos() + curve->m_frames[i].first;
				vec3 P1 = p_obj->m_transform.get_pos() + curve->m_frames[i + 3].first;

				mat4 model = glm::translate(mat4(1.0f), P0);
				model = glm::scale(model, vec3(0.5f));
				shader->set_uniform("M", model);
				renderer->get_model("sphere")->draw(shader, nullptr);

				model = glm::translate(mat4(1.0f), P1);
				model = glm::scale(model, vec3(0.5f));
				shader->set_uniform("M", model);
				renderer->get_model("sphere")->draw(shader, nullptr);
			}
		}
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
			vec3 m_att_factor = m_lights[1]->m_ldata.m_att_factor;
			if (ImGui::InputFloat3("ALL Diffuse", &m_diffuse.x))
				for (auto l : m_lights)
					l->m_ldata.m_diffuse = m_diffuse;
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
				m_objects[i]->draw_GUI();
				if (ImGui::Button("Delete Object"))
				{
					delete m_objects[i];
					m_objects.erase(m_objects.begin() + i);
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
