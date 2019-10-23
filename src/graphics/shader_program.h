/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	shader_program.h
Purpose: Program/Shader Interface
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#pragma once
#include <glm/glm.h>
#include <string>
#include <map>
class Shader_Program
{
public:
	Shader_Program(const std::string& vtx, const std::string& frag);
	Shader_Program(const std::string& vtx, const std::string& geo, const std::string& frag);
	Shader_Program(const std::string& vtx, const std::string& tess_control, const std::string& tess_eval, const std::string& frag);
	Shader_Program(const std::string& vtx, const std::string& tess_control, const std::string& tess_eval, const std::string& geo, const std::string& frag);
	~Shader_Program();

	bool is_valid()const;
	void recompile();
	void use()const;
	void set_uniform(const char *name, bool val) const;
	void set_uniform(const char *name, int val) const;
	void set_uniform(const char *name, float val) const;
	void set_uniform(const char *name, const vec3 & v) const;
	void set_uniform(const char *name, const vec4 & v) const;
	void set_uniform(const char *name, const mat3 & m) const;
	void set_uniform(const char *name, const mat4 & m) const;
	void set_uniform_sampler(const int & val) const;
	void set_uniform_subroutine(unsigned int shader_type, const std::string& value);
	std::string paths[5]{};

private:
	enum class e_shader_type { VERTEX, TESS_CONTROL, TESS_EVAL, GEOMETRY, FRAGMENT};
	bool create_handle();
	void compile_program();
	bool compile_shader(const std::string& filename, const e_shader_type& type);
	void link();
	int uniform_location(const char * name) const;

	std::map<std::string, unsigned int> m_subroutine_values;
	mutable std::map<std::string, int> m_uniform_location_map{};
	int m_handle{0};
	bool m_linked{false};
};