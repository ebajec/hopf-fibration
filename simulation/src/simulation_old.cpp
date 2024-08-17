#include "simulation_old.h"
#include "camera.h"
#include "imgui.h"
#include "defines.h"
#include "shader.h"
#include "simulation.h"
#include "ui.h"
#include <memory>
#include <vector>


/**********************************************************************************
 * 
 * Implementation details for HopfSimulation.
 * 
 **********************************************************************************/
HopfSimulation::HopfSimulation(const char* title, int width, int height, int x, int y) : 
    BaseViewWindow(title, width, height,x,y,NULL,NULL),
    ui(m_window),
    shaderManager(std::make_shared<ShaderManager>("../graphics/shader/")),
    m_controller(shaderManager),
    m_hopfDisplay(shaderManager)
{   
    m_controlViewport = Viewport(
        UI_PARAMS_PANEL_WIDTH, 
        UI_PARAMS_PANEL_WIDTH, 
        ivec2(m_width - UI_PARAMS_PANEL_WIDTH,m_height - UI_PARAMS_PANEL_WIDTH),
        Camera(vec3(1,0,0),vec3(-2,0,0)));
    m_controlViewport.name = "S2";
    m_controlViewport.setRenderCallback(this,HopfSimulation::controlViewportRenderCallback);

    m_sceneViewport = Viewport(
        m_width - UI_PARAMS_PANEL_WIDTH, 
        m_height, 
        ivec2(0,0),
        Camera(vec3(-1,-1,-1),vec3(5,5,5)));
    m_sceneViewport.name = "Hopf Fibration";
    m_sceneViewport.setRenderCallback(this, HopfSimulation::sceneViewportRenderCallback);
   

     if (!initShaders()) {
        printf("ERROR: Failed to initialize shaders.\n");
        return;
    }
    initFiberData();
    windowLoop();
}

bool HopfSimulation::initShaders() {
    // Initialize shader programs and camera
    shaderManager->addProgram(
    "default",           
    {"default.vert","solid_color.frag"});
    shaderManager->addProgram(
    "blinn-phong",       
    {"default.vert","blinnphong.frag"});
    shaderManager->addProgram(
    "blinn-phong-instanced", 
    {"spheres_instanced.vert","blinnphong.frag"});
    shaderManager->addProgram(
    "polyline",              
    {"polyline_mesh.comp"});
    shaderManager->addProgram(
    "hopf",                  
    {"hopf.comp"});
    shaderManager->addProgram(
    "spheres_transform",                
    {"spheres_transform.comp"});
    return true;
}   

bool HopfSimulation::initFiberData() 
{
    std::vector<SpherePointData> points(FIBER_COUNT);

    auto spherePath = [](float t) {
        t = 2*PI*t;
        float s = PI;
        return vec3(sin(t)*cos(s),sin(t)*sin(s),cos(t));
    };

    for (unsigned int i = 0; i < FIBER_COUNT; i++)
    {
        float t = (float)i/(float)FIBER_COUNT;
        points[i].position = vec4(spherePath(t),1.0f);
    }

    m_controller.uploadPointData(points.data(),points.size()*sizeof(SpherePointData));
    m_hopfDisplay.setPoints(m_controller.getPoints());
    m_hopfDisplay.updateIndexData(FIBER_COUNT, FIBER_SIZE);
    return true; 
}

void HopfSimulation::updateScene()
{
    m_camera.updateUbo();
    m_controller.updateBallPositions();
    m_hopfDisplay.updateFiberData();
}

void HopfSimulation::sceneViewportRenderCallback(void* usr, Camera& camera)
{
    HopfSimulation* ctx = static_cast<HopfSimulation*>(usr);
    ctx->m_hopfDisplay.render(camera);
}

void HopfSimulation::controlViewportRenderCallback(void* usr, Camera& camera)
{
    HopfSimulation* ctx = static_cast<HopfSimulation*>(usr);
    ctx->m_controller.render(camera);
}

void HopfSimulation::renderUI()
{
    m_sceneViewport.fixPos(ivec2(0,0));
    m_sceneViewport.fixSize(ivec2(m_width - UI_PARAMS_PANEL_WIDTH, m_height));
    m_controlViewport.fixPos(ivec2(m_width - UI_PARAMS_PANEL_WIDTH,m_height - UI_PARAMS_PANEL_WIDTH));
    m_controlViewport.fixSize(ivec2(UI_PARAMS_PANEL_WIDTH,UI_PARAMS_PANEL_WIDTH));

    // Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Parameters controls
    ImGui::SetNextWindowPos(ImVec2(m_width - UI_PARAMS_PANEL_WIDTH, 0));
    ImGui::SetNextWindowSize(ImVec2(UI_PARAMS_PANEL_WIDTH, m_height - UI_PARAMS_PANEL_WIDTH));

	ImGui::Begin("Parameters", nullptr, ImGuiWindowFlags_NoResize);                          
	//ImGui::SliderInt("Circle count", &this->hf_draw_max, 0, FIBER_COUNT);
	ImGui::End();

    m_controlViewport.add(&ui);
    m_sceneViewport.add(&ui);

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());	
}
void HopfSimulation::windowLoop()
{
    printf("-----------------------------\n");
    printf("Press ESC to toggle GUI access\n");
    printf("Use WASD and mouse to move\n");

    while (!glfwWindowShouldClose(m_window)) 
    {
    	glfwGetFramebufferSize(m_window, &m_width, &m_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        updateScene();

        //Rendering
        renderUI();
        m_controlViewport.render();
        m_sceneViewport.render();

    	glfwSwapBuffers(m_window);    
        glfwPollEvents();
    }
}