
#include "simulation.h"
#include "misc.h"
#include "defines.h"
#include "ui.h"

Simulation::Simulation(const char* title, int width, int height) : BaseViewWindow(title, width, height,NULL,NULL) 
{
    m_ui = new ImguiContextGLFW(m_window);

    if (!initShaders()) {
        printf("ERROR: Failed to initialize shaders.\n");
        return;
    }

    printf("-----------------------------\n");
    printf("Press ESC to toggle GUI access\n");
    printf("Use WASD and mouse to move\n");

    windowLoop();    
}
Simulation::~Simulation() {
    
}
bool Simulation::initShaders() 
{
    return false;
}   

void Simulation::renderUI()
{
    // Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	// Parameters controls
	ImGui::Begin("Parameters");                          
	//ImGui::SliderFloat("Animation speed", &this->hf_anim_speed, 0.0f, 1.0f);
	//ImGui::SliderInt("Circle count", &this->hf_draw_max, 0, FIBER_COUNT);
	ImGui::End();
	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());	
}
void Simulation::windowLoop()
{
    glDepthFunc(GL_LESS);
    
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
    	glfwGetFramebufferSize(m_window, &m_width, &m_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Rendering
        renderUI();

    	glfwSwapBuffers(m_window);    
    }

    
}