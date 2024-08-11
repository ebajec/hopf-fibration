#ifndef SIMULATION_H
#define SIMULATION_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "meshgen.h"
#include "renderer.h"
#include "defines.h"
#include "objects.h"
#include "window.h"
#include "shader.h"


/**********************************************************************************
 * 
 * Control sphere manager
 * 
 **********************************************************************************/
#define SPHERE_SIZE 128
#define SCOUNT 100
class PointController{
public:
    PointController();
    void render(ShaderProgram& shader, const std::vector<vec4>& points,int limit);
    void transform(mat4 trans);
private:

    mat4 geometry = mat4(1.0f);
    std::vector<vec3> points;
    std::vector<vec3> pointColors;
    Sphere* sphere;
};

/**********************************************************************************
 * 
 * Main window/program.  Renders hopf fibration and includes UI to control
 * some parameters.
 * 
 * Some things to note:
 * - the "hf's" stands for "hopf fibration"
 * - the "c's" stand for "control"
  * 
 **********************************************************************************/
class HopfSimulation : public BaseViewWindow {
public:
    HopfSimulation(const char* title, int width, int height, int x, int y);
    ~HopfSimulation();

protected:
    void windowLoop();
    bool initFiberData();
    bool initControllerData();
    bool initShaders();

    void updatePositions();

    void renderFibers();
    void renderSpheres();
    void renderUI();

    ShaderManager * shaderManager;

    Buffer spherePoints;
    Buffer lineInstances;
    PrimitiveData<LineData> hopfCircles;

    mat4 model = mat4(1.0f);

    std::vector<vec4> spherePointsCPU;

    PointController* m_controller;
    Camera* c_cam;
    Sphere* c_sphere;

    MultiIndex hf_indices;
    int hf_draw_max = FIBER_COUNT;                  // Parameter to control number of drawn circles
};




#endif