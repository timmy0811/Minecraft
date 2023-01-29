#include "Shader.h"

int Shader::GetUniformLocation(const std::string& name) const
{
    if (m_UniformLacationCache.find(name) != m_UniformLacationCache.end())
        return m_UniformLacationCache[name];

    GLCall(int location = glGetUniformLocation(m_RendererID, name.c_str()));
    if (location == -1)
        std::cout << "Warning: uniform: " << name << " cannot be found." << std::endl;

    m_UniformLacationCache[name] = location;

    return location;
}

Shader::Shader(const std::string& path_vert, const std::string& path_frag)
    :m_RendererID(0), m_PathVert(path_vert), m_PathFrag(path_frag)
{
    ShaderProgramSource source = ParseShader(path_vert, path_frag);
    m_RendererID = CreateShader(source.VertexSource, source.FragmentSource);
}

Shader::~Shader()
{
    GLCall(glDeleteProgram(m_RendererID));
}

void Shader::Bind() const
{
    GLCall(glUseProgram(m_RendererID));
}

void Shader::Unbind() const
{
    GLCall(glUseProgram(0));
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
    int pos = GetUniformLocation(name);
    GLCall(glUniform4f(pos, v0, v1, v2, v3));
}

void Shader::SetUniform1f(const std::string& name, float v0)
{
    GLCall(glUniform1f(GetUniformLocation(name), v0));
}

void Shader::SetUniform1i(const std::string& name, int v0)
{
    GLCall(glUniform1i(GetUniformLocation(name), v0));
}

void Shader::SetUniform2f(const std::string& name, float v0, float v1)
{
    GLCall(glUniform2f(GetUniformLocation(name), v0, v1));
}

void Shader::SetUniform1iv(const std::string& name, unsigned int size, int* v)
{
    GLCall(glUniform1iv(GetUniformLocation(name), size, v));
}

void Shader::SetUniform1fv(const std::string& name, unsigned int size, float* v)
{
    GLCall(glUniform1fv(GetUniformLocation(name), size, v));
}

void Shader::SetUniform3fv(const std::string& name, unsigned int size, glm::vec3* v)
{
    GLCall(glUniform3fv(GetUniformLocation(name), size, &v->x));
}

void Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2)
{
    GLCall(glUniform3f(GetUniformLocation(name), v0, v1, v2));
}

void Shader::SetUniformMaterial(const std::string& name, OpenGL::Material& m)
{
    GLCall(glUniform3f(GetUniformLocation(name + ".ambient"), m.ambient.r, m.ambient.g, m.ambient.b));
    GLCall(glUniform3f(GetUniformLocation(name + ".diffuse"), m.diffuse.r, m.diffuse.g, m.diffuse.b));
    GLCall(glUniform3f(GetUniformLocation(name + ".specular"), m.specular.r, m.specular.g, m.specular.b));
    GLCall(glUniform1f(GetUniformLocation(name + ".shine"), m.shine));
}

void Shader::SetUniformConstantLight(const std::string& name, OpenGL::ConstantLight& m)
{
    GLCall(glUniform3f(GetUniformLocation(name + ".position"), m.position.r, m.position.g, m.position.b));
    GLCall(glUniform3f(GetUniformLocation(name + ".ambient"), m.ambient.r, m.ambient.g, m.ambient.b));
    GLCall(glUniform3f(GetUniformLocation(name + ".diffuse"), m.diffuse.r, m.diffuse.g, m.diffuse.b));
    GLCall(glUniform3f(GetUniformLocation(name + ".specular"), m.specular.r, m.specular.g, m.specular.b));
}

void Shader::SetUniformSpotLight(const std::string& name, OpenGL::Spotlight& m)
{
    GLCall(glUniform3f(GetUniformLocation(name + ".position"), m.position.r, m.position.g, m.position.b));
    GLCall(glUniform3f(GetUniformLocation(name + ".direction"), m.direction.r, m.direction.g, m.direction.b));
    GLCall(glUniform1f(GetUniformLocation(name + ".cutOff"), m.cutOff));
    GLCall(glUniform1f(GetUniformLocation(name + ".outerCutOff"), m.outerCutOff));

    GLCall(glUniform3f(GetUniformLocation(name + ".ambient"), m.ambient.r, m.ambient.g, m.ambient.b));
    GLCall(glUniform3f(GetUniformLocation(name + ".diffuse"), m.diffuse.r, m.diffuse.g, m.diffuse.b));
    GLCall(glUniform3f(GetUniformLocation(name + ".specular"), m.specular.r, m.specular.g, m.specular.b));

    GLCall(glUniform1f(GetUniformLocation(name + ".constant"), m.constant));
    GLCall(glUniform1f(GetUniformLocation(name + ".linear"), m.linear));
    GLCall(glUniform1f(GetUniformLocation(name + ".quadratic"), m.quadratic));
}

void Shader::SetUniformPointLight(const std::string& name, OpenGL::PointLight& m)
{
    GLCall(glUniform3f(GetUniformLocation(name + ".position"), m.position.r, m.position.g, m.position.b));
    GLCall(glUniform3f(GetUniformLocation(name + ".ambient"), m.ambient.r, m.ambient.g, m.ambient.b));
    GLCall(glUniform3f(GetUniformLocation(name + ".diffuse"), m.diffuse.r, m.diffuse.g, m.diffuse.b));
    GLCall(glUniform3f(GetUniformLocation(name + ".specular"), m.specular.r, m.specular.g, m.specular.b));

    GLCall(glUniform1f(GetUniformLocation(name + ".constant"), m.constant));
    GLCall(glUniform1f(GetUniformLocation(name + ".linear"), m.linear));
    GLCall(glUniform1f(GetUniformLocation(name + ".quadratic"), m.quadratic));
}

void Shader::SetUniformDirectionalLight(const std::string& name, OpenGL::DirectionalLight& m)
{
    GLCall(glUniform3f(GetUniformLocation(name + ".direction"), m.direction.r, m.direction.g, m.direction.b));
    GLCall(glUniform3f(GetUniformLocation(name + ".ambient"), m.ambient.r, m.ambient.g, m.ambient.b));
    GLCall(glUniform3f(GetUniformLocation(name + ".diffuse"), m.diffuse.r, m.diffuse.g, m.diffuse.b));
    GLCall(glUniform3f(GetUniformLocation(name + ".specular"), m.specular.r, m.specular.g, m.specular.b));
}

void Shader::SetUniformConstantLight(const std::string& name, OpenGL::ConstantLight& m, unsigned int index)
{
    GLCall(glUniform3f(GetUniformLocation(name + "[" + std::to_string(index) + "]" + ".position"), m.position.r, m.position.g, m.position.b));
    GLCall(glUniform3f(GetUniformLocation(name + "[" + std::to_string(index) + "]" + ".ambient"), m.ambient.r, m.ambient.g, m.ambient.b));
    GLCall(glUniform3f(GetUniformLocation(name + "[" + std::to_string(index) + "]" + ".diffuse"), m.diffuse.r, m.diffuse.g, m.diffuse.b));
    GLCall(glUniform3f(GetUniformLocation(name + "[" + std::to_string(index) + "]" + ".specular"), m.specular.r, m.specular.g, m.specular.b));
}

void Shader::SetUniformSpotLight(const std::string& name, OpenGL::Spotlight& m, unsigned int index)
{
    GLCall(glUniform3f(GetUniformLocation(name + "[" + std::to_string(index) + "]" + ".position"), m.position.r, m.position.g, m.position.b));
    GLCall(glUniform3f(GetUniformLocation(name + "[" + std::to_string(index) + "]" + ".direction"), m.direction.r, m.direction.g, m.direction.b));
    GLCall(glUniform1f(GetUniformLocation(name + "[" + std::to_string(index) + "]" + ".cutOff"), m.cutOff));
    GLCall(glUniform1f(GetUniformLocation(name + "[" + std::to_string(index) + "]" + ".outerCutOff"), m.outerCutOff));

    GLCall(glUniform3f(GetUniformLocation(name + "[" + std::to_string(index) + "]" + ".ambient"), m.ambient.r, m.ambient.g, m.ambient.b));
    GLCall(glUniform3f(GetUniformLocation(name + "[" + std::to_string(index) + "]" + ".diffuse"), m.diffuse.r, m.diffuse.g, m.diffuse.b));
    GLCall(glUniform3f(GetUniformLocation(name + "[" + std::to_string(index) + "]" + ".specular"), m.specular.r, m.specular.g, m.specular.b));

    GLCall(glUniform1f(GetUniformLocation(name + "[" + std::to_string(index) + "]" + ".constant"), m.constant));
    GLCall(glUniform1f(GetUniformLocation(name + "[" + std::to_string(index) + "]" + ".linear"), m.linear));
    GLCall(glUniform1f(GetUniformLocation(name + "[" + std::to_string(index) + "]" + ".quadratic"), m.quadratic));
}

void Shader::SetUniformPointLight(const std::string& name, OpenGL::PointLight& m, unsigned int index)
{
    GLCall(glUniform3f(GetUniformLocation(name + "[" + std::to_string(index) + "]" + ".position"), m.position.r, m.position.g, m.position.b));
    GLCall(glUniform3f(GetUniformLocation(name + "[" + std::to_string(index) + "]" + ".ambient"), m.ambient.r, m.ambient.g, m.ambient.b));
    GLCall(glUniform3f(GetUniformLocation(name + "[" + std::to_string(index) + "]" + ".diffuse"), m.diffuse.r, m.diffuse.g, m.diffuse.b));
    GLCall(glUniform3f(GetUniformLocation(name + "[" + std::to_string(index) + "]" + ".specular"), m.specular.r, m.specular.g, m.specular.b));

    GLCall(glUniform1f(GetUniformLocation(name + "[" + std::to_string(index) + "]" + ".constant"), m.constant));
    GLCall(glUniform1f(GetUniformLocation(name + "[" + std::to_string(index) + "]" + ".linear"), m.linear));
    GLCall(glUniform1f(GetUniformLocation(name + "[" + std::to_string(index) + "]" + ".quadratic"), m.quadratic));
}

void Shader::SetUniformDirectionalLight(const std::string& name, OpenGL::DirectionalLight& m, unsigned int index)
{
    GLCall(glUniform3f(GetUniformLocation(name + "[" + std::to_string(index) + "]" + ".direction"), m.direction.r, m.direction.g, m.direction.b));
    GLCall(glUniform3f(GetUniformLocation(name + "[" + std::to_string(index) + "]" + ".ambient"), m.ambient.r, m.ambient.g, m.ambient.b));
    GLCall(glUniform3f(GetUniformLocation(name + "[" + std::to_string(index) + "]" + ".diffuse"), m.diffuse.r, m.diffuse.g, m.diffuse.b));
    GLCall(glUniform3f(GetUniformLocation(name + "[" + std::to_string(index) + "]" + ".specular"), m.specular.r, m.specular.g, m.specular.b));
}

void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& mat)
{
    int idex = GetUniformLocation(name);
    GLCall(glUniformMatrix4fv(idex, 1, GL_FALSE, &mat[0][0]));
}

ShaderProgramSource Shader::ParseShader(const std::string path_vertex, const std::string path_frag) {
    const std::string& A = StringFromPath(path_vertex);
    const std::string& B = StringFromPath(path_frag);
    return { A, B };
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source) {
    GLCall(unsigned int id = glCreateShader(type));
    const char* src = source.c_str();
    GLCall(glShaderSource(id, 1, &src, nullptr));
    GLCall(glCompileShader(id));

    int result;
    GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    if (result == GL_FALSE) {
        int length;
        GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        char* message = (char*)alloca(length * sizeof(char));
        GLCall(glGetShaderInfoLog(id, length, &length, message));
        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl; 
        std::cout << message << std::endl;
        GLCall(glDeleteShader(id));
    }

    return id;
}

const std::string Shader::StringFromPath(const std::string& path)
{
    std::ifstream stream(path);

    std::string line;
    std::stringstream ss;
    bool isValid = false;

    while (getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) {
            isValid = true;
        }
        else if(isValid) {
            ss << line << "\n";
        }
    }
    return ss.str();
}

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
    GLCall(unsigned int program = glCreateProgram());
    GLCall(unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader));
    GLCall(unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader));

    GLCall(glAttachShader(program, vs));
    GLCall(glAttachShader(program, fs));
    GLCall(glLinkProgram(program));
    GLCall(glValidateProgram(program));

    GLCall(glDeleteShader(vs));
    GLCall(glDeleteShader(fs));

    return program;
}
