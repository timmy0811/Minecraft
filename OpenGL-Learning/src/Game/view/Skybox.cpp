#include "Skybox.h"

Skybox::Skybox(const std::string& cubemapPathPrefix, const std::string& fileFormat)
	:m_RendererID(0), m_FilepathPrefix(cubemapPathPrefix),
    m_Shader("res/shaders/skybox/shader_skybox.vert", "res/shaders/skybox/shader_skybox.frag")
{
    m_Shader.Bind();

    GLCall(glActiveTexture(GL_TEXTURE0));

	GLCall(glGenTextures(1, &m_RendererID));
	GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID));

    int width, height, nrChannels;
    unsigned char* data;
    for (unsigned int i = 0; i < 6; i++)
    {
        stbi_set_flip_vertically_on_load(0);
        data = stbi_load((cubemapPathPrefix + '_' + std::to_string(i) + fileFormat).c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
            stbi_image_free(data);
        }
        else {
            std::cout << "Could not load image " + cubemapPathPrefix + '_' + std::to_string(i) + fileFormat << '\n';
        }
    }

    GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    m_VB = std::make_unique<VertexBuffer>(skyboxVertices, sizeof(skyboxVertices));  // ???
    m_VA = std::make_unique<VertexArray>();

    m_VBLayout = std::make_unique<VertexBufferLayout>();
    m_VBLayout->Push<float>(3);	// Position

    m_VA->AddBuffer(*m_VB, *m_VBLayout);

    m_Shader.Unbind();
}

Skybox::~Skybox()
{
    GLCall(glDeleteTextures(1, &m_RendererID));
}

void Skybox::Unbind()
{
    GLCall(glActiveTexture(m_BoundID));
    GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
    m_BoundID = -1;
}

const unsigned int Skybox::Bind(const unsigned int slot)
{
    m_Shader.Bind();

    GLCall(glActiveTexture(GL_TEXTURE0 + slot));
    GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID));
    m_BoundID = static_cast<int>(slot);

    m_Shader.SetUniform1i("u_Cubemap", slot);

    return slot;
}

void Skybox::OnRender()
{
    m_DrawCalls++;

    GLCall(glDepthFunc(GL_LEQUAL));
    Renderer::DrawArray(*m_VA, m_Shader, 0, 36);
    GLCall(glDepthFunc(GL_LESS));
}

void Skybox::setMatrix(const glm::mat4& projection, const glm::mat4& view)
{
    m_Shader.Bind();
    m_Shader.SetUniformMat4f("u_ViewTest", glm::mat4(glm::mat3(view)));
    m_Shader.SetUniformMat4f("u_Projection", projection);
}
