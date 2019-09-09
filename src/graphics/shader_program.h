#pragma once
#include <glm/glm.h>
#include <string>
#include <map>
class Shader_Program
{
public:
	Shader_Program(const std::string& vtx, const std::string& frag);
	Shader_Program(const std::string& vtx, const std::string& geo, const std::string& frag);
	~Shader_Program();

	void use()const;
	void set_uniform(const char *name, bool val) const;
	void set_uniform(const char *name, int val) const;
	void set_uniform(const char *name, float val) const;
	void set_uniform(const char *name, const vec3 & v) const;
	void set_uniform(const char *name, const vec4 & v) const;
	void set_uniform(const char *name, const mat3 & m) const;
	void set_uniform(const char *name, const mat4 & m) const;
	void set_uniform_sampler(const char *name, const int & val) const;

private:
	enum class e_shader_type { VERTEX, FRAGMENT, GEOMETRY };
	bool create_handle();
	bool compile_shader(const std::string& filename, const e_shader_type& type);
	void link();
	int uniform_location(const char * name) const;

	mutable std::map<std::string, int> m_uniform_location_map{};
	int m_handle{0};
	bool m_linked{false};
};