#include <memory>
#include <vector>

#include "hopf.h"
#include "misc.h"
#include "defines.h"
#include "mesh.h"
#include "shader.h"
#include "renderer.h"
#include "simulation_old.h"


/**********************************************************************************
 * 
 * Implementation details for SphereController
 * 
 **********************************************************************************/
SphereController::SphereController(const std::shared_ptr<ShaderManager>& shaderManager):
 m_shaderManager(shaderManager)
{
    auto sphereParam = [](float u, float v)
    {
        return S2(2*PI*v,PI*u);
    };

    auto meshData = meshFromSurface(sphereParam,64,64,1,0);
    m_sphereMesh.uploadData(meshData.first,meshData.second,GL_STREAM_DRAW);
}

void SphereController::render(Camera& camera)
{   
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);

    auto temp = m_shaderManager.lock();

    ShaderProgram shader = temp->program("blinn-phong");
    ShaderProgram instanceShader= temp->program("blinn-phong-instanced");

    size_t indexCount = m_sphereMesh.indexCount();
    size_t pointCount = points.size()/sizeof(SpherePointData);

    camera.bindUbo(0);

    m_sphereMesh.bind();

    // Render the big sphere
    shader.use();
    shader.setUniform("model",geometry,GL_TRUE);
    shader.setUniform("scale",1.0f);
    //glDrawElements(GL_TRIANGLES,indexCount,GL_UNSIGNED_INT,(void*)0);

    // Render the little balls
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,0,points.id());
    instanceShader.use();
    instanceShader.setUniform("model",geometry,true);
    instanceShader.setUniform("scale",0.04f);
    glDrawElementsInstanced(GL_TRIANGLES,indexCount,GL_UNSIGNED_INT,(void*)0,pointCount);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,0,0);

    m_sphereMesh.unbind();
}
void SphereController::transform(mat4 trans) 
{
    geometry = geometry*trans;
}

void SphereController::updateBallPositions()
{
    ShaderProgram computePositions = m_shaderManager.lock()->program("spheres_transform");

    uint sphereCount = (uint)(points.size()/sizeof(SpherePointData));

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,0,points.id());
    computePositions.use();
    computePositions.setUniform("count",sphereCount);
    computePositions.setUniform("rand",uRand());
    computePositions.dispatchCompute(sphereCount, 1, 1);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,0,0);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void SphereController::uploadPointData(void* data, size_t size)
{
    this->points.uploadData(data,size,GL_STREAM_DRAW);
}

/**********************************************************************************
 * 
 * Implementation details for HopfHController.
 * 
 **********************************************************************************/

 HopfFibrationDisplay::HopfFibrationDisplay(const std::shared_ptr<ShaderManager>& shaderManager) :
 m_shaderManager(shaderManager)
 {
    hopfCircles.reserveAttribData(FIBER_COUNT*FIBER_SIZE);
    hopfCircles.attribPointer(0, 4, GL_FLOAT, GL_FALSE, 0);
    hopfCircles.attribPointer(1, 4, GL_FLOAT, GL_FALSE, (void*)sizeof(vec4));  
    updateIndexData(0, 0);
 }

 void HopfFibrationDisplay::updateIndexData(const uint fiberCount, const uint fiberRes)
 {
    std::vector<InstanceLine> instances(fiberCount);
    for (unsigned int i = 0; i < fiberCount; i++)
    {
        instances[i] = 
        {
            .cmd = {
            .count = fiberRes,
            .instanceCount = 1,
            .first = i*fiberRes,
            .baseInstance = 0
            },
            .width = 1.0f,
            .padding = 0
        };
    }

    lineInstances.uploadData(instances);
 }

 void HopfFibrationDisplay::updateFiberData()
 {
    ShaderProgram hopf_map = m_shaderManager.lock()->program("hopf");
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,0,spherePoints->id());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,1,lineInstances.id());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,2,hopfCircles.vbo()->id());

    hopf_map.use();
    hopf_map.setUniform("numFibers",(unsigned int)FIBER_COUNT);
    hopf_map.dispatchCompute(FIBER_COUNT, FIBER_SIZE, 1);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,0,0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,1,0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,2,0);
 }

void HopfFibrationDisplay::render(Camera& camera)
{
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA);

    ShaderProgram shader = m_shaderManager.lock()->program("default");

    camera.bindUbo(0);

    shader.use();
    shader.setUniform("model",mat4(1.0f),GL_FALSE);
    shader.setUniform("scale",1.0f);
    
    hopfCircles.bind();
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER,lineInstances.id());
    glMultiDrawArraysIndirect(GL_LINE_LOOP,0,hf_draw_max,sizeof(InstanceLine));
    hopfCircles.unbind();

    glBindBufferBase(GL_UNIFORM_BUFFER,0,0);
}

void HopfFibrationDisplay::setPoints(const Buffer& points)
{
    this->spherePoints = &points;
}
