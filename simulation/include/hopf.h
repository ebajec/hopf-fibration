#ifndef HOPF_H
#define HOPF_H

#include <memory>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "mesh.h"
#include "renderer.h"
#include "defines.h"
#include "shader.h"


/**********************************************************************************
 * 
 * Control sphere manager
 * 
 **********************************************************************************/

struct SpherePointData
{
    vec4 position;
    vec4 color;
};

struct InstanceLine
{
    DrawArraysIndirectCommand cmd;
    float width;
    uint padding;
};

class SphereController
{
public:
    SphereController(const std::shared_ptr<ShaderManager>& shaderManager);

    void updateBallPositions();
    void render(Camera& camera);
    void transform(mat4 trans);
    const Buffer& getPoints() {return points;}
    void uploadPointData(void* data, size_t size);

private:
    Buffer points;
    Mesh m_sphereMesh;
    Camera m_camera;
    mat4 geometry = mat4(1.0f);

    int pointCount;

    std::weak_ptr<ShaderManager> m_shaderManager;
};

class HopfFibrationDisplay
{
public:
    HopfFibrationDisplay(const std::shared_ptr<ShaderManager>& shaderManager);

    void updateIndexData(const uint fiberCount, const uint fiberRes);
    void updateFiberData();
    void render(Camera& camera);

    void setPoints(const Buffer& points);
private:
    const Buffer* spherePoints;
    Buffer lineInstances;
    PrimitiveData<LineData> hopfCircles;

    std::weak_ptr<ShaderManager> m_shaderManager;
    int hf_draw_max = FIBER_COUNT;   
};

#endif