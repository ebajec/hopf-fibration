#include "renderer.h"

Buffer::Buffer() : m_size(0) , m_id(0)
{
    glGenBuffers(1,&m_id);
}

Buffer::~Buffer()
{
    glDeleteBuffers(1,&m_id);
}

void Buffer::uploadData(void* data, size_t size, GLenum usage)
{
    glBindBuffer(GL_ARRAY_BUFFER,m_id);
    if (size < m_size)
        glBufferSubData(GL_ARRAY_BUFFER,0,size,data);
    else 
        glBufferData(GL_ARRAY_BUFFER,size,data,usage);
    m_size = size;
}

Renderer::Renderer() 
{
}

void Renderer::setShaderManager(ShaderManager *shaderManager)
{
    shaders = shaderManager;
}

void Renderer::renderMesh(Camera *camera, PrimitiveData<Vertex> mesh)
{
    camera->bindUbo(0);
}