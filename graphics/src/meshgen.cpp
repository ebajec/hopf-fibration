#include "meshgen.h"

MeshGen::MeshGen()
{
}

MeshGen::~MeshGen()
{
}

void MeshGen::setShaderManager(ShaderManager *shaderManager)
{
    shaders = shaderManager;
}

void MeshGen::polylineMesh(Buffer& lines, Buffer& instances, PrimitiveData<Vertex>& mesh, uint resolution)
{
    uint numLines = instances.size()/sizeof(LineInstance);
    uint totalPoints = lines.size()/sizeof(LineData);
    uint detail = resolution;

	//set up bindings on GPU
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER,0,lines.id());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER,1,instances.id());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER,2,tempData.id());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,3,mesh.vbo()->id());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,4,mesh.ebo()->id());

    uint localSizeX = 32;
    uint localSizeY = 16;
    uint localSizeZ = 1;

    uint nGroupsX = totalPoints/localSizeX;
    uint nGroupsY = detail/localSizeY;
    uint nGroupsZ = numLines/localSizeZ ;

    ShaderProgram shader = shaders->getProgram("polyline_mesh");

    shader.setUniform("numLines",numLines);
    shader.setUniform("lineDetail",detail);
    shader.use();

	glDispatchCompute(nGroupsX,nGroupsY,nGroupsZ);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
}
