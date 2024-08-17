#ifndef SIMULATION_H
#define SIMULATION_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>

#include "hopf.h"
#include "ui.h"
#include "window.h"
#include "shader.h"

#define UI_PARAMS_PANEL_WIDTH 600
#define UI_PARAMS_PANEL_HEIGHT 200

class HopfSimulationUI;

/**********************************************************************************
 * 
 * Main window/program.  Renders hopf fibration and includes UI to control
 * some parameters.
 * 
 **********************************************************************************/

class HopfSimulation : public BaseViewWindow {
public:
    HopfSimulation(const char* title, int width, int height, int x, int y);

protected:
    void windowLoop();

    bool initFiberData();
    bool initShaders();

    void updatePositions();
    void updateScene();

    void renderUI();

    static void controlViewportRenderCallback(void* usr, Camera& camera);
    static void sceneViewportRenderCallback(void* usr, Camera& camera);

    std::shared_ptr<ShaderManager> shaderManager;

    HopfFibrationDisplay  m_hopfDisplay;
    SphereController      m_controller;
    
    int a;
    ImGuiContextGLFW ui;

    Viewport m_controlViewport, m_sceneViewport;

    //int padding[2];
};






#endif