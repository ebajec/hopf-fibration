#include "meshgen.h"

MultiIndex indicesFromInstance(Buffer& instances)
{
    uint count = instances.size()/sizeof(LineInstance);

    BufferMap<LineInstance> instanceData(instances,0, count, GL_MAP_READ_BIT);

    std::vector<int> indices(2*count);

    for (uint i = 0; i < count; i++)
    {
        indices[i]         = instanceData[i].firstIndex;
        indices[count + i] = instanceData[i].size;
    }

    return {indices};
}
    
std::vector<vec4> surfacePoints(
    vec3 (*param)(float, float), const int uCount, const int vCount)
{
    std::vector<vec4> points(uCount*vCount);
    float u,v;
    size_t indexCount;

    for (int i = 0; i < uCount; i++)
    {
        for (int j = 0; j < vCount; j++)
        {
            u = (float)i/(float)uCount;
            v = (float)j/(float)vCount;

            vec4 p = vec4(param(u,v),1);
            points[i*uCount + j] = p;
        }
    }
    return points;
}

static int nextIndex(const int chi, const int index, const char coord)
{
    switch (coord)
    {
    case 0:
        switch (chi)
        {

        }
    case 1:
        switch (chi)
        {

        }
    default: 
        return 0;
    }
}

std::vector<int> surfaceIndices(const int uCount, const int vCount, const int chi)
{
    std::vector<int> indices(6*uCount*vCount);

    size_t counter = 0;

    for (int i = 0; i < uCount; i++)
    {
        for (int j = 0; j < vCount; j++)
        {
            indices[counter++] = i*vCount + j;
            indices[counter++] = i*vCount + j + 1;
            indices[counter++] = (i+1)*vCount + j;
            indices[counter++] = i*vCount + j;
            indices[counter++] = i*vCount + j + 1;
            indices[counter++] = (i+1)*vCount + j;
        }
    }
}

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
