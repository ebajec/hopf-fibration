#include "simulation_old.h"
#include "imgui.h"
#include "misc.h"
#include "defines.h"
#include "meshgen.h"
#include "shader.h"
#include "simulation.h"
#include "renderer.h"
#include "timer.h"

#define SHOW_BUFFER(data, size, id)\
glBindBuffer(GL_ARRAY_BUFFER,id);\
float data[size];\
{\
    void * out = glMapBufferRange(GL_ARRAY_BUFFER,0,size*sizeof(float), GL_MAP_READ_BIT);\
    memcpy(data,out,size*sizeof(float));\
    glUnmapBuffer(GL_ARRAY_BUFFER);\
    glBindBuffer(GL_ARRAY_BUFFER,0);\
}

/**********************************************************************************
 * 
 * Implementation details for SphereController
 * 
 **********************************************************************************/
PointController::PointController()
{
    sphere = new Sphere(SPHERE_SIZE,SPHERE_SIZE,vec3{1,0,0});
    points = vector<vec3>(SCOUNT);

    for (int i = 0; i < SCOUNT; i++) 
        points[i] = S2(uRand()*2*PI,uRand()*PI);
    sphere->setScale(0.07f);
}

void PointController::render(ShaderProgram& shader, const vector<vec4>& inpoints, int limit)
{   
    int i = 0;
    int size = inpoints.size();
    for (vec4 p : inpoints)
    {
        float t = (float)i/(float)size;
        shader.setUniform("hsv",vec3{t,0.8,0.7});
        sphere->setPos(vec3(geometry*p));   
        sphere->render(shader);
        i++;

        if (i > limit) break;
    }
}
void PointController::transform(mat4 trans) 
{
    geometry = geometry*trans;
}


/**********************************************************************************
 * 
 * Implementation details for HopfHopfSimulation.
 * 
 **********************************************************************************/
HopfSimulation::HopfSimulation(const char* title, int width, int height) : BaseViewWindow(title, width, height,NULL,NULL) 
{
    shaderManager = new ShaderManager("../graphics/shader/");
    windowLoop();
}
HopfSimulation::~HopfSimulation() {

    delete shaderManager;
    delete c_cam;
    delete c_sphere;
    delete m_controller;
}
bool HopfSimulation::initShaders() {
    // Initialize shader programs and camera
    shaderManager->addProgram("default",     {"default.vert","solid_color.frag"});
    shaderManager->addProgram("blinn-phong", {"default.vert","blinnphong.frag"});
    shaderManager->addProgram("polyline",    {"polyline_mesh.comp"});
    shaderManager->addProgram("hopf",        {"hopf.comp"});
    shaderManager->addProgram("m_mult",      {"matrix_transform.comp"});

    return true;
}   

vec3 spherePath(float t) {
    t = 2*PI*t;
    float s = 4*PI;//sin(t)*t + PI*cos(2*t) + 10;
    return vec3{sin(t)*cos(s),sin(t)*sin(s),cos(t)};
}

bool HopfSimulation::initFiberData() 
{
    // Prepare buffers for computing fibers of hopf map

    hopfCircles.reserveAttribData(FIBER_COUNT*FIBER_SIZE);
    hopfCircles.attribPointer(0, 4, GL_FLOAT, GL_FALSE, 0);
    hopfCircles.attribPointer(1, 4, GL_FLOAT, GL_FALSE, (void*)sizeof(vec4));
    
    std::vector<vec4> points(FIBER_COUNT);
    std::vector<LineInstance> instances(FIBER_COUNT);

    for (unsigned int i = 0; i < FIBER_COUNT; i++)
    {
        float t = (float)i/(float)FIBER_COUNT;
        points[i] = vec4(spherePath(t),1.0f);
        instances[i] = { FIBER_SIZE,i*FIBER_SIZE,1.0f,0};
    }

    spherePointsCPU = points;
    spherePoints.uploadData(points);
    lineInstances.uploadData(instances);

    hf_indices = indicesFromInstance(lineInstances);
    hopfCircles.setMultiDrawIndices(hf_indices);

    return true; 
}
bool HopfSimulation::initControllerData() 
{
    c_cam = new Camera(vec3(-1,0,0),vec3(2,0,0),CONTROL_VIEWPORT_SIZE,CONTROL_VIEWPORT_SIZE,PI/4);
    c_sphere = new Sphere(128,128);
    m_controller = new PointController();
    return true;
}

void HopfSimulation::renderFibers() 
{   
    
    ShaderProgram hopf_map = shaderManager->getProgram("hopf");
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,0,spherePoints.id());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,1,lineInstances.id());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,2, hopfCircles.vbo()->id());

    hopf_map.use();
    hopf_map.setUniform("numFibers",(unsigned int)FIBER_COUNT);
    hopf_map.dispatchCompute(FIBER_COUNT, FIBER_SIZE, 1);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

    // Render hopf fibration in main viewport
    glViewport(0, 0, m_width, m_height);
    m_camera.updateUbo();
    m_camera.bindUbo(0);

    glDepthMask(GL_TRUE);

    ShaderProgram shader = shaderManager->getProgram("default");
    shader.use();
    shader.setUniform("model",mat4(1.0f),GL_FALSE);
    shader.setUniform("scale",1.0f);

    int c = 0;

    glEnable(GL_DEPTH_TEST);

    hopfCircles.bind();
    glMultiDrawArrays(GL_LINE_LOOP,hf_indices.firsts()+c,hf_indices.counts()+c,hf_draw_max-c);
    hopfCircles.unbind();

    glBindBufferBase(GL_UNIFORM_BUFFER,0,0);
}

void HopfSimulation::renderSpheres()
{   
    glClear(GL_DEPTH_BUFFER_BIT);
    // Render sphere in bottom right corner
    glViewport(m_width - CONTROL_VIEWPORT_SIZE, 0, CONTROL_VIEWPORT_SIZE, CONTROL_VIEWPORT_SIZE);
    c_cam->updateUbo();
    c_cam->bindUbo(0);

    ShaderProgram light = shaderManager->getProgram("blinn-phong");
    ShaderProgram temp = shaderManager->getProgram("default");

    light.use();
    glEnable(GL_BLEND);
    c_sphere->render(temp);

    light.use();
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    m_controller->render(light,spherePointsCPU,hf_draw_max);   
    glBindBufferBase(GL_UNIFORM_BUFFER,0,0);
}

void HopfSimulation::updatePositions()
{
    ShaderProgram matrixMult = shaderManager->getProgram("m_mult");
    uint sphereCount = (uint)(spherePoints.size()/sizeof(vec4));

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,0,spherePoints.id());

    matrixMult.use();
    matrixMult.setUniform("matrix",model,GL_FALSE);
    matrixMult.setUniform("count",sphereCount);
    matrixMult.setUniform("rand",uRand());
    matrixMult.dispatchCompute(sphereCount, 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void HopfSimulation::renderUI()
{
    // Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	// Parameters controls
    ImGui::SetNextWindowPos(ImVec2(0, m_height - 200)); // Position at (100, 100)
    ImGui::SetNextWindowSize(ImVec2(300, 200)); // Size (300x200)
	ImGui::Begin("Parameters", nullptr, ImGuiWindowFlags_NoResize);                          
	ImGui::SliderInt("Circle count", &this->hf_draw_max, 0, FIBER_COUNT);
	ImGui::End();
	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());	
}
void HopfSimulation::windowLoop()
{
    printf("-----------------------------\n");
    printf("Press ESC to toggle GUI access\n");
    printf("Use WASD and mouse to move\n");

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glDepthFunc(GL_LESS);

    initFiberData();
    initControllerData();
    
    if (!initShaders()) {
        printf("ERROR: Failed to initialize shaders.\n");
        return;
    }


    while (!glfwWindowShouldClose(m_window)) 
    {
        glfwPollEvents();
    	glfwGetFramebufferSize(m_window, &m_width, &m_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_camera.resize(m_width, m_height);


        size_t count = spherePointsCPU.size();
        BufferMap<vec4> sphereData(spherePoints,0,count,GL_MAP_READ_BIT);
        memcpy(spherePointsCPU.data(),sphereData.data,count*sizeof(vec4));
        
        m_controller->transform(model);

        updatePositions();

        //Rendering
        renderFibers();
        renderSpheres();
        renderUI();

    	glfwSwapBuffers(m_window);    
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}