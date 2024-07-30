#ifndef RENDERER_H
#define RENDERER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "shader.h"
#include "camera.h"


/*************************************************************************
 * 
 * typedefs/structs
 * 
 *************************************************************************/

typedef unsigned int uint;

typedef enum 
{
    Integral,
    Float,
    Double
} AttributeType;

struct AttribLayout
{
    GLuint         location;   // Location of attribute in vertex array  
    GLint          size;       // Number of components in attribute (i.e., for vec4 this would be 4).
    GLenum         type;       // Data type.
    GLboolean      normalized; // Whether to normalize float data.
    const void*    pointer;    // Byte offset from beginning of VBO to first occurence of this attribute.
    AttributeType  aType;      // Specifies which version of glVertexAttribPointer to use.
};

struct Vertex
{
    vec4 position;
    vec4 normal;
    vec4 color;
};

struct MultiIndex
{
    uint size;       // Length of starts and counts
    uint * starts;   // starting indices
    uint * counts;
};


/*************************************************************************
 * 
 * Buffer: Class Defintion
 * 
 *************************************************************************/

class Buffer
{
public:
    Buffer();
    ~Buffer();

    template<typename T>
    void uploadData(vector<T> data, GLenum usage = GL_STREAM_DRAW);
    void uploadData(void * data, size_t size, GLenum usage = GL_STREAM_DRAW);

    const size_t size() const {return m_size;}
    const GLuint id() const {return m_id;}

private:
    GLuint m_id;
    size_t m_size;
};

/*************************************************************************
 * 
 * PrimitiveData: Class Defintion
 * 
 *************************************************************************/

template<typename vType>
class PrimitiveData
{
public:
    PrimitiveData();
    ~PrimitiveData();

    void uploadData(const vector<vType>& data, const vector<uint>& indices);
    bool setAttribLayout(const vector<AttribLayout> & attribs);
    void bind() const; 
    void unbind() const;

    const Buffer * const vbo(){return &m_vbo;}
    const Buffer * const ebo(){return &m_ebo;}

private:
    bool setAttribPointer(const AttribLayout & attrib,uint location);
    GLuint m_vao;
    Buffer m_vbo;
    Buffer m_ebo;
};

/*************************************************************************
 * 
 * Renderer: Class Defintion
 * 
 *************************************************************************/

class Renderer
{
public:
    Renderer();
    ~Renderer() {}
    void setShaderManager(ShaderManager* shaderManager);

    void renderMesh(Camera* camera, PrimitiveData<Vertex> mesh);
protected:
    ShaderManager * shaders;
};

/**
 * Buffer: Implementation of templates
 */

template <typename T>
inline void Buffer::uploadData(vector<T> data, GLenum usage)
{
    size_t size = data.size()*sizeof(T);
    glBindBuffer(GL_ARRAY_BUFFER,m_id);

    if (size < m_size)
    {
        glBufferSubData(GL_ARRAY_BUFFER,0,size,data.data());   
    } 
    else 
    {
        glBufferData(GL_ARRAY_BUFFER,size,data.data(),usage);  
    }
    m_size = size;
    
}

/*************************************************************************
 * 
 * PrimitiveData: Implementation of templates
 * 
 *************************************************************************/

template<typename vType>
PrimitiveData<vType>::PrimitiveData()
{
    glGenVertexArrays(1,&m_vao);
}

template<typename vType>
PrimitiveData<vType>::~PrimitiveData()
{
    glDeleteVertexArrays(1,&m_vao);
}

template<typename vType>
void PrimitiveData<vType>::uploadData(const vector<vType>& data, const vector<uint>& indices)
{
    m_vbo.uploadData((void*)data.data(),data.size()*sizeof(vType));
    m_ebo.uploadData((void*)indices.data(),indices.size()*sizeof(uint));
}

template <typename vType>
bool PrimitiveData<vType>::setAttribLayout(const vector<AttribLayout> &attribs)
{
    for (uint i = 0; i < attribs.size(); i++)
        if (!setAttribPointer(attribs[i],i))
            return false;
    return true;
}

template <typename vType>
inline void PrimitiveData<vType>::bind() const
{
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER,m_vbo.id());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_ebo.id());
}

template <typename vType>
inline void PrimitiveData<vType>::unbind() const
{
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
}

template <typename vType>
inline bool PrimitiveData<vType>::setAttribPointer(const AttribLayout &attrib, uint location)
{
    GLenum type = attrib.type;
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER,m_vbo.id());
    switch (attrib.aType)
    {
    case Float:
        glVertexAttribPointer(location,attrib.size,type,attrib.normalized,sizeof(vType),attrib.pointer);
        return true;
    case Integral:
        glVertexAttribIPointer(location,attrib.size,type,sizeof(vType),attrib.pointer);
        return true;
    case Double:
        glVertexAttribLPointer(location,attrib.size,type,sizeof(vType),attrib.pointer);
        return true;
    default:
        return false;
    }
}

#endif


