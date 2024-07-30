#ifndef SIMULATION_H
#define SIMULATION_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "matrix.h"

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
    void render(ShaderProgram& shader, const vector<vec4>& points);
    void transform(mat4 trans);
private:

    mat4 geometry = mat4::id();
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
    HopfSimulation(const char* title, int width, int height);
    ~HopfSimulation();

protected:
    void windowLoop();
    bool initFiberData();
    bool initControllerData();
    bool initShaders();
    void renderFibers();
    void renderControlSphere();
    void renderUI();

    ShaderManager * shaderManager;

    Buffer spherePoints;
    Buffer lineInstances;

    vector<vec4> spherePointsCPU;

    PointController* m_controller;
    Camera* c_cam;
    Sphere* c_sphere;

    ShaderProgram c_shader;
    ShaderProgram hf_main_shader;                   // Handles rendering of main scene 
    ShaderProgram hf_map;                           // Computes fibers of hopf map

    GLuint hf_vao;                               
    GLuint hf_vbo_out; 
    GLfloat hf_data_in[8*FIBER_COUNT*FIBER_SIZE];   // Arguments to hf_map; allows parallel computation of fibers.
    GLint hf_draw_firsts[FIBER_COUNT];              // For glMultiDrawArrays
    GLsizei hf_draw_counts[FIBER_COUNT];            // For glMultiDrawArrays               
    int hf_draw_max = FIBER_COUNT;                  // Parameter to control number of drawn circles
};




#endif