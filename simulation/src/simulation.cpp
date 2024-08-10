
#include "simulation.h"
#include "misc.h"
#include "defines.h"
#include "shader.h"
#include "ui.h"

Simulation::Simulation(const char* title, int width, int height) : BaseViewWindow(title, width, height,NULL,NULL) 
{
    m_ui = new ImguiContextGLFW(m_window);
    m_shaderManager = new ShaderManager("../graphics/shaders/");

    printf("-----------------------------\n");
    printf("Press ESC to toggle GUI access\n");
    printf("Use WASD and mouse to move\n");

    windowLoop();    
}
Simulation::~Simulation() 
{
}

void Simulation::windowLoop()
{
    glDepthFunc(GL_LESS);
    
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
    	glfwGetFramebufferSize(m_window, &m_width, &m_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    	glfwSwapBuffers(m_window);    
    }

    
}