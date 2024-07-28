#include "renderer_base.h"

bool BaseRenderer::setActiveShader(Shader& shader)
{
    if (!shader.isLinked())
        return false;
        
    this->activeShader = &shader;
    return true;
}
