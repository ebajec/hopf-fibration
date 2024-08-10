#ifndef SIMULATION_H
#define SIMULATION_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "defines.h"
#include "window.h"
#include "shader.h"
#include "ui.h"
#include "renderer.h"

/**********************************************************************************
 * 
 * Main window/program.  
* 
 **********************************************************************************/
class Simulation : public BaseViewWindow {
public:
    Simulation(const char* title, int width, int height);
    ~Simulation();

protected:
    void windowLoop();

    ShaderManager * m_shaderManager;
    ImguiContextGLFW * m_ui;
};

#endif