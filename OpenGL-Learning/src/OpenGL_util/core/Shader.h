#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <unordered_map>
#include "glm/glm.hpp"

#include "../debug/Debug.hpp"
#include "../misc/Material.hpp"
#include "../misc/Light.hpp"

struct ShaderProgramSource {
	std::string VertexSource;
	std::string FragmentSource;
};

class Shader
{
private:
	unsigned int m_RendererID;
	const std::string& m_PathVert;
	const std::string& m_PathFrag;

	mutable std::unordered_map<std::string, int> m_UniformLacationCache {};

	int GetUniformLocation(const std::string& name) const;

	const std::string StringFromPath(const std::string& path);
	unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
	unsigned int CompileShader(unsigned int type, const std::string& source);
	ShaderProgramSource ParseShader(const std::string path_vertex, const std::string path_frag);

public:
	Shader(const std::string& path_vert, const std::string& path_frag);
	~Shader();

	void Bind() const;
	void Unbind() const;

	void SetUniform1i(const std::string& name, int v0);

	void SetUniform1f(const std::string& name, float v0);
	void SetUniform2f(const std::string& name, float v0, float v1);
	void SetUniform3f(const std::string& name, float v0, float v1, float v2);
	void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);

	void SetUniform1iv(const std::string& name, unsigned int size, int* v);

	void SetUniform1fv(const std::string& name, unsigned int size, float* v);
	void SetUniform3fv(const std::string& name, unsigned int size, glm::vec3* v);

	void SetUniformMaterial(const std::string& name, OpenGL::Material& m);

	void SetUniformConstantLight(const std::string& name, OpenGL::ConstantLight& m);
	void SetUniformSpotLight(const std::string& name, OpenGL::Spotlight& m);
	void SetUniformPointLight(const std::string& name, OpenGL::PointLight& m);
	void SetUniformDirectionalLight(const std::string& name, OpenGL::DirectionalLight& m);

	void SetUniformConstantLight(const std::string& name, OpenGL::ConstantLight& m, unsigned int index);
	void SetUniformSpotLight(const std::string& name, OpenGL::Spotlight& m, unsigned int index);
	void SetUniformPointLight(const std::string& name, OpenGL::PointLight& m, unsigned int index);
	void SetUniformDirectionalLight(const std::string& name, OpenGL::DirectionalLight& m, unsigned int index);

	void SetUniformMat4f(const std::string& name, const glm::mat4& mat);
};

