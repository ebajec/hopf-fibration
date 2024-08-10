#ifndef RENDERER_LINE_H
#define RENDERER_LINE_H

#include "renderer.h"
#include "shader.h"

struct LineData 
{
    vec4 position;
    vec4 color;
};

struct TangentFrame
{
    vec4 T; 
    vec4 N;
    vec4 B;
};

struct LineInstance
{
    uint size;
    uint firstIndex;
    float width;
    uint padding;
};

MultiIndex indicesFromInstance(Buffer& instances);

std::vector<vec4> surfacePoints(vec3 param(float u, float v), const int uCount, const int vCount);
std::vector<int> surfaceIndices(const int uCount, const int vCount, const int chi);

class MeshGen 
{
public:
    MeshGen();
    ~MeshGen();

    void setShaderManager(ShaderManager* shaderManager);

    void polylineMesh(Buffer& lines, Buffer& instances, PrimitiveData<Vertex>& mesh, uint resolution = 10);
private:
    ShaderManager * shaders;
    Buffer tempData;
};

#endif
