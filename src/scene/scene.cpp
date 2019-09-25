#include "scene.h"
#include <fstream>
#include <sstream>
#include <imgui/imgui.h>
#include <utils/math_utils.h>
#include <graphics/renderer.h>
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

			// Clean Scene
			m_objects.clear();

			// Create Objects
			{
				size_t obj_location = stream.find("objects");
				std::string objs = stream.substr(obj_location);
				objs = objs.substr(objs.find_first_of('[') + 1, objs.find_first_of(']') - objs.find_first_of('['));
				while (!objs.empty())
				{
					std::string obj = objs.substr(0, objs.find_first_of(0x1D));
					objs = objs.substr(objs.find_first_of(0x1D)+2);

					std::string mesh_name = obj.substr(obj.find("mesh") + 5, obj.find_first_of(',') - obj.find("mesh") - 5);

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

					vec3 scl{ -1.0 };
					obj = obj.substr(obj.find("scale"));
					scl.x = (float)std::atof(obj.substr(obj.find_first_of('x') + 2, obj.find_first_of('y') - obj.find_first_of('x') - 3).c_str());
					scl.y = (float)std::atof(obj.substr(obj.find_first_of('y') + 2, obj.find_first_of('z') - obj.find_first_of('y') - 3).c_str());
					scl.z = (float)std::atof(obj.substr(obj.find_first_of('z') + 2, obj.find_first_of('}') - obj.find_first_of('z') - 2).c_str());

					transform3d tr;
					tr.set_tr(pos, scl, rot);
					
					m_objects.push_back(new scene_object(mesh_name, tr));
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
	tr.set_scl(vec3{ .5f });
	int num_lights = 20;
	light_data ld;
	for (int i = 0; i < num_lights; i++)
	{
		tr.set_pos({ random_float(17.5f, 32.5f), random_float(0.f, 50.f),random_float(-106.f, 94.f) });
		ld.m_diffuse = { random_float(0.3f, 1.f), random_float(0.3f, 1.f),random_float(0.3f, 1.f) };
		m_lights.push_back(new light(tr, ld));
	}

	for (int i = 0; i < num_lights; i++)
	{
		tr.set_pos({ random_float(-27.5f, -12.5f), random_float(0.f, 50.f),random_float(-106.f, 94.f) });
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
	tr.set_scl(vec3{ 0.5f });

	for (auto p_li : m_lights)
	{
		tr.set_pos(p_li->m_transform.get_pos());
		shader->set_uniform("M", tr.get_model());
		renderer->get_model("sphere")->draw(shader);
	}
}

void c_scene::shutdown()
{
	for (auto p_obj : m_objects)
		delete p_obj;
	m_objects.clear();
}

void c_scene::drawGUI()
{
	ImGui::Text("# Objects #");
	if (ImGui::TreeNode("Scene Options"))
	{
		if (ImGui::Button("Reload"))
			load_scene(m_scene_name);
		ImGui::Checkbox("Animate Scene", &m_animated_scene);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Objects List"))
	{
		if (ImGui::Button("Create"))
			m_objects.push_back(new scene_object{ "cube" });
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
				if (ImGui::DragFloat3("Rotation", &m_objects[i]->m_transform.m_tr.m_rot.x))chng = true;
				if (ImGui::DragFloat3("Scale", &m_objects[i]->m_transform.m_tr.m_scl.x, .1f, .001f, 99999999.f))chng = true;
				if (chng)m_objects[i]->m_transform.m_tr.upd();
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
				scene->m_lights[i]->m_ldata.drawGUI();

				if (ImGui::DragFloat3("Position", &scene->m_lights[i]->m_transform.m_tr.m_pos.x, .1f))
					scene->m_lights[i]->m_transform.m_tr.upd(); if (ImGui::Button("Delete"))
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
