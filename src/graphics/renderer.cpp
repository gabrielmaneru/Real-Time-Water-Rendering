#include "renderer.h"
#include "gl_error.h"
#include <platform/window_manager.h>
#include <scene/scene.h>
#include <scene/shape.h>
#include <GL/gl3w.h>
#include <iostream>
#include <algorithm>

c_renderer* renderer = new c_renderer;
constexpr int MAX_STEPS = 100;
constexpr float MAX_DIST = 100.f;
constexpr float SURF_DIST = .01f;
float c_renderer::get_dist(vec3 p)
{
	float dist{ MAX_DIST };
	for (auto& pObj : scene->m_objects)
	{
		float newdist = dynamic_cast<BaseShape*>(pObj)->get_distance(p);
		if (dist == MAX_DIST)
			dist = newdist;
		else
			dist = glm::min<float>(dist, newdist);
	}
	return dist;
}
float c_renderer::ray_march(vec3 cam_eye, vec3 cam_dir)
{
	float dO = 0.0f;
	for (int i = 0; i < MAX_STEPS; i++)
	{
		vec3 p = cam_eye + cam_dir * dO;
		float ds = get_dist(p);
		if (ds < SURF_DIST)
			break;

		dO += ds;
		if (dO >= MAX_DIST)
			break;
	}
	return dO;
}
vec3 c_renderer::get_normal(vec3 p)
{
	float d = get_dist(p);
	vec3 n = d - vec3{
		get_dist(p - vec3{.01f, .0f, .0f}),
		get_dist(p - vec3{.0f, .01f, .0f}),
		get_dist(p - vec3{.0f, .0f, .01f}),
	};
	if (glm::length2(n) == 0.0f)
		return n;
	return glm::normalize(n);
}
float c_renderer::get_light(vec3 p)
{
	vec3 lightPos{ 4, 5, 6 };
	vec3 l = glm::normalize(lightPos - p);
	vec3 n = get_normal(p);

	float dif = glm::clamp(glm::dot(n, l), 0.0f, 1.0f);

	float d = ray_march(p + n * SURF_DIST * 2.0f, l);
	if (d < glm::length(lightPos - p))
		dif *= 0.1f;
	return dif;
}
bool c_renderer::init()
{
	if (gl3wInit())
		return false;
	
	if (!gl3wIsSupported(4, 0))
		return false;

	// GL Options
	GL_CALL(glClearColor(0.1f, 0.1f, 0.1f, 1.0f));

	//Load Programs
	try { shader = new Shader_Program("resources/shaders/color.vert", "resources/shaders/color.frag"); }
	catch (const std::string & log) { std::cout << log; return false; }

	//Load Resources
	render_texture.setup(window_manager->get_width(), window_manager->get_height());
	render_texture.clear({ 0.5f, 0.1f, 0.2f });
	cam.eye = { 0.0f, 0.0f, 1.0f };
	cam.target = { 0.0f, 0.0f, 0.0f };
	cam.view_rect = { 0.0f, (float)render_texture.m_width, 0.0f, (float)render_texture.m_height };
	model_texture.set_tr({ 0.0f, 0.0f }, { cam.view_rect.y, cam.view_rect.w });

	//Load Meshes
	{ // Load Quad
		std::vector<vec3> positions = {
					{-0.5f, -0.5f, 0.0f},
					{0.5f,  -0.5f, 0.0f},
					{0.5f,  0.5f,  0.0f},

					{-0.5f, 0.5f,  0.0f},
					{-0.5f, -0.5f, 0.0f},
					{0.5f,  0.5f,  0.0f},
		};

		for (auto& pos : positions) {
			pos.x += 0.5f;
			pos.y += 0.5f;
			pos.z += 0.5f;
		}

		std::vector<vec2> uvs = {
				{0.0f, 0.0f},
				{1.0f, 0.0f},
				{1.0f, 1.0f},
				{0.0f, 1.0f},
				{0.0f, 0.0f},
				{1.0f, 1.0f},
		};

		// Vertices
		uint32_t vbo_vertices{};
		GL_CALL(glGenBuffers(1, &vbo_vertices));
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices));
		GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * positions.size(), positions.data(), GL_STATIC_DRAW));

		// UV
		uint32_t vbo_uv{};
		GL_CALL(glGenBuffers(1, &vbo_uv));
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo_uv));
		GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * uvs.size(), uvs.data(), GL_STATIC_DRAW));

		// VAO
		uint32_t vao{};
		GL_CALL(glGenVertexArrays(1, &vao));
		GL_CALL(glBindVertexArray(vao));

		// Positions
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices));
		GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr));
		GL_CALL(glEnableVertexAttribArray(0));

		// UV
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo_uv));
		GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, nullptr));
		GL_CALL(glEnableVertexAttribArray(1));
		GL_CALL(glBindVertexArray(0));

		quad.vao = vao;
		quad.cnt = 6;
	}
	return true;
}

void c_renderer::update()
{
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	// Camera Update
	cam.update();

	// Scene Draw
	vec3 cam_eye{ 0.0f, 0.0f, 1.0f };
	float fov = 90.0f;
	float fov_ratio_h = fov / 90.0f;
	float fov_ratio_v = fov_ratio_h * render_texture.m_height / render_texture.m_width;
	
	for (size_t y = 0; y < render_texture.m_height; ++y)
		for (size_t x = 0; x < render_texture.m_width; ++x)
		{
			vec3 cam_dir{ 
				map(x, (size_t)0, render_texture.m_width, -fov_ratio_h, fov_ratio_h),
				map(y, (size_t)0, render_texture.m_height, -fov_ratio_v, fov_ratio_v),
				-1.0f
			};
			cam_dir = glm::normalize(cam_dir);
			float d = ray_march(cam_eye, cam_dir);
			vec3 p = cam_eye + cam_dir * d;
			vec3 col{ get_light(p) };
			render_texture.set(x, y, col);
		}

	// Load Quad texture
	render_texture.load();
	
	// Set shader
	shader->use();

	mat4 mvp = cam.proj * cam.view * model_texture.m_tr.get_model();
	shader->set_uniform("MVP", mvp);

	GL_CALL(glActiveTexture(GL_TEXTURE0));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, render_texture.m_id));
	GL_CALL(glUniform1i(render_texture.m_id, 0));

	// Bind mesh
	GL_CALL(glBindVertexArray(quad.vao));

	// Draw fill
	GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
	GL_CALL(glDrawArrays(GL_TRIANGLES, 0, quad.cnt));
}

void c_renderer::shutdown()
{
	//Clean Resouces

}
