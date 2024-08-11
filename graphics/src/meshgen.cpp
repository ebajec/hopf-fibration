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
    

void surfaceParamMeshIndex(int uCount, int vCount, int i, int j, int b1, int b2, size_t& counter, std::vector<int>& indices)
{
    if (b1 == 0 && b2 == 0)
    {
        int iNext = i+1;
        int jNext = j+1;

        if (iNext == uCount || jNext == vCount) return;

        indices[counter++] = i*vCount + j;
        indices[counter++] = i*vCount + jNext;
        indices[counter++] = iNext*vCount + j;
        indices[counter++] = i*vCount + j;
        indices[counter++] = i*vCount + jNext;
        indices[counter++] = iNext*vCount + j;
        return;
    }
    if (b1 == 1 && b2 == 0)
    {
        int iNext = i+1;
        int jNext = (j >= uCount - 1) ? 0 : j+1;

        if (iNext == uCount) return;

        indices[counter++] = i*vCount + j;
        indices[counter++] = i*vCount + jNext;
        indices[counter++] = iNext*vCount + j;
        indices[counter++] = i*vCount + j;
        indices[counter++] = i*vCount + jNext;
        indices[counter++] = iNext*vCount + j;
    }
    if (b1 == 0 && b2 == 1)
    {
        return;
    }
    if (b1 == 2 && b2 == 1)
    {
        int iNext = (i >= uCount - 1) ? 0 : i+1;
        int jNext = (j >= uCount - 1) ? 0 : j+1;

        indices[counter++] = i*vCount + j;
        indices[counter++] = i*vCount + jNext;
        indices[counter++] = iNext*vCount + j;
        indices[counter++] = i*vCount + j;
        indices[counter++] = i*vCount + jNext;
        indices[counter++] = iNext*vCount + j;
    return;
    }
    return;
}

std::vector<vec4> surfacePoints(
    vec3 (*param)(float, float), const int uCount, const int vCount, const int b1, const int b2)
{
    std::vector<vec4> points(uCount*vCount);
    std::vector<int> indices(6*(uCount+1)*(vCount+1));
    float u,v;
    size_t indexCount = 0;

    for (int i = 0; i < uCount; i++)
    {
        for (int j = 0; j < vCount; j++)
        {
            u = (float)i/(float)uCount;
            v = (float)j/(float)vCount;

            vec4 p = vec4(param(u,v),1);
            points[i*uCount + j] = p;

            surfaceParamMeshIndex(uCount,vCount,i,j,b1,b2,indexCount,indices);
        }
    }
    return points;
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
