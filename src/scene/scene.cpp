#include "scene.h"
#include <fstream>
#include <sstream>
#include <imgui/imgui.h>
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
	tr.set_pos({ 23.0f, 3.0f, -90.0f });
	tr.set_scl(vec3{.5f });
	m_lights.push_back(new light(tr, light_data{}));
	return true;
}

void c_scene::update()
{
}

void c_scene::draw_obj(Shader_Program * shader)
{
	for (auto p_obj : m_objects)
		p_obj->draw(shader);
}

void c_scene::draw_light(Shader_Program * shader)
{
	shader->set_uniform("la", light_data::m_ambient);
	for (auto p_li : m_lights)
		p_li->draw(shader);
}

void c_scene::shutdown()
{
	for (auto p_obj : m_objects)
		delete p_obj;
	m_objects.clear();
}

void c_scene::drawGUI()
{
	ImGui::Text("Objects");
	for (int i = 0; i < m_objects.size(); i++)
	{
		ImGui::PushID(i);
		std::string tree_name = m_objects[i]->m_model
			? m_objects[i]->m_model->m_name
			: "Unknown Object";

		if (ImGui::TreeNode(tree_name.c_str()))
		{
			bool chng{ false };
			if (ImGui::DragFloat3("Pos", &scene->m_objects[i]->m_transform.m_tr.m_pos.x, .1f))chng = true;
			if (ImGui::DragFloat3("Rot", &scene->m_objects[i]->m_transform.m_tr.m_rot.x))chng = true;
			if (ImGui::DragFloat3("Scl", &scene->m_objects[i]->m_transform.m_tr.m_scl.x, .1f, .001f))chng = true;
			if (chng)scene->m_objects[i]->m_transform.m_tr.upd();
			ImGui::TreePop();
		}
		ImGui::PopID();
	}


	ImGui::NewLine();
	ImGui::Text("Lights");
	static bool display_break{ false };
	ImGui::Checkbox("", &display_break);
	ImGui::SameLine();
	if (display_break)
		ImGui::DragFloat3("Ambi", &light_data::m_ambient.x, 0.01f, 0.0f, 1.0f);
	else if (ImGui::DragFloat("Ambi", &light_data::m_ambient.x, 0.01f, 0.0f, 1.0f))
		light_data::m_ambient.y = light_data::m_ambient.z = light_data::m_ambient.x;

	ImGui::SliderFloat("AttMax", &light_data::m_att_max, 0.001f, 1.0f);
	ImGui::InputFloat("radius", &scene->m_lights[0]->m_transform.m_tr.m_scl.x);
	for (int i = 0; i < m_lights.size(); i++)
	{
		ImGui::PushID(i);

		if (ImGui::TreeNode("Light"))
		{
			scene->m_lights[i]->m_ldata.drawGUI();

			if (ImGui::DragFloat3("Pos", &scene->m_lights[i]->m_transform.m_tr.m_pos.x, .1f))
				scene->m_lights[i]->m_transform.m_tr.upd();
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
}
