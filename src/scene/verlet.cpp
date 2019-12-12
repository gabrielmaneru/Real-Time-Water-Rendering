#include "verlet.h"
#include <graphics/renderer.h>
#include <graphics/model.h>

cloth::cloth(transform3d tr)
	:scene_object("", tr, nullptr, nullptr)
{
	m_mesh.build_plane(50, 1.0f);
	m_mesh.compute_normals();
}

void cloth::draw(Shader_Program * shader)
{
	shader->set_uniform("M", m_transform.m_tr.get_model());
	Model::m_def_materials[1]->set_uniform(shader);
	m_mesh.load();
	m_mesh.draw();
}

void cloth::draw_GUI()
{
}
