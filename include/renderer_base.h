#ifndef RENDERER_H
#define RENDERER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <unordered_map>
#include "misc.h"
#include "matrix.h"
#include "shader.h"

/*************************************************************************
 * 
 * typedefs/structs
 * 
 *************************************************************************/

typedef unsigned int uint;

struct AttribLayout
{
    GLuint      location;   // Location of attribute in vertex array  
    GLint       size;       // Number of components in attribute (i.e., for vec4 this would be 4).
    GLenum      type;       // Data type.
    GLboolean   normalized; // Whether to normalize float data.
    const void* pointer;    // Byte offset from beginning of VBO to first occurence of this attribute.
};

struct Vertex
{
    vec4 position;
    vec4 normal;
    vec4 color;
};

/*************************************************************************
 * 
 * BaseRenderer: Class Defintion
 * 
 *************************************************************************/

class BaseRenderer
{
public:
    BaseRenderer() {};
    ~BaseRenderer() {};

    virtual void render() = 0;

    bool setActiveShader(Shader& shader);
private:
    Shader * activeShader = nullptr;
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

    void bindArray() {glBindVertexArray(vao);}
    void bindData() {glBindBuffer(GL_ARRAY_BUFFER,vbo);}
    void bindIndices() {glBindBuffer(GL_ARRAY_BUFFER,ebo);}
private:
    bool setAttribPointer(const AttribLayout & attrib,uint location);
    GLuint vao, vbo, ebo;  
};


/*************************************************************************
 * 
 * PrimitiveData: Implementation of templates
 * 
 *************************************************************************/

template<typename vType>
PrimitiveData<vType>::PrimitiveData()
{
    glGenVertexArrays(1,&vao);
    glGenBuffers(1,&vbo);
    glGenBuffers(1,&ebo);
}

template<typename vType>
PrimitiveData<vType>::~PrimitiveData()
{
    glDeleteVertexArrays(1,&vao);
    glDeleteBuffers(1,&vbo);
    glDeleteBuffers(1,&ebo);
}

template<typename vType>
void PrimitiveData<vType>::uploadData(const vector<vType>& data, const vector<uint>& indices)
{
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER,data.size()*sizeof(vType),data.data(),GL_STREAM_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,ebo);
    glBufferData(GL_ARRAY_BUFFER,indices.size()*sizeof(uint),indices.data(),GL_STREAM_DRAW);
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
inline bool PrimitiveData<vType>::setAttribPointer(const AttribLayout &attrib, uint location)
{
    GLenum type = attrib.type;
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    if (isFloat(type))
        glVertexAttribPointer(location,attrib.size,type,attrib.normalized,sizeof(vType),attrib.pointer);
    else if (isIntegral(type))
        glVertexAttribIPointer(location,attrib.size,type,sizeof(vType),attrib.pointer);
    else if (type == GL_DOUBLE)
        glVertexAttribLPointer(location,attrib.size,type,sizeof(vType),attrib.pointer);
    else 
        return false;
    return true;
}


#endif