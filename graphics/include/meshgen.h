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
    float width;
    uint size;
    uint firstIndex;
    uint padding;
};

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

//Pretend main method. 

// Renderer renderer;
// ComputeManager;

// // Only inputs
// Buffer spherePoints;
// Buffer circleData;
// Buffer circleInstances;

// // Intermediate data. Could be stored outside of main program logic.
// Buffer lineTangentFrames;
// PrimitiveData<Vertex> lineMeshes;

// // Only done on update. 
// ComputeHopf(spherePoints,circles);
// // Computations like this which have intermediate steps could hold their own cache.
// ComputeLineMeshes(circleData, circleIndices  ,lineMeshes);


// renderer.setActiveShader(defaultMesh);
// lineMeshes.bind();
// renderer.render();
// lineMeshes.unbind();

