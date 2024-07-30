#include "simulation_old.h"
#include "misc.h"
#include "defines.h"
#include "meshgen.h"
#include "simulation.h"

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

void PointController::render(ShaderProgram& shader, const vector<vec4>& inpoints)
{   
    for (vec4 p : inpoints)
    {
        sphere->setPos(vec3(geometry*vec4(p)));   
        sphere->render(shader);
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
    glDeleteBuffers(1,&hf_vbo_out);
    glDeleteVertexArrays(1,&hf_vao);

    delete shaderManager;
    delete c_cam;
    delete c_sphere;
    delete m_controller;
}
bool HopfSimulation::initShaders() {
    // Initialize shader programs and camera
    shaderManager->addProgram("hf_main",  {"default_camera.vert","default_color.frag"});
    shaderManager->addProgram("cw_main",  {"control_viewport.vert","control_viewport.frag"});
    shaderManager->addProgram("polyline", {"polyline_mesh.comp"});
    shaderManager->addProgram("hopf",     {"hopf.comp"});

    hf_main_shader = shaderManager->getProgram("hf_main");
    c_shader = shaderManager->getProgram("cw_main");

    return true;
}   

vec3 spherePath(float t) {
    t = 2*PI*t;
    float s = PI/4.0f;//sin(t)*t + PI*cos(2*t) + 10;
    return vec3{sin(t)*cos(s),sin(t)*sin(s),cos(t)};
}

bool HopfSimulation::initFiberData() 
{
    // Set up linearly spaced inputs for fiber calculation.
    for (int i = 0; i < FIBER_COUNT; i++) {
        hf_draw_firsts[i] = i*FIBER_SIZE;
        hf_draw_counts[i] = FIBER_SIZE;  
    }

    // Prepare buffers for computing fibers of hopf map
    glGenVertexArrays(1, &hf_vao);
    glBindVertexArray(hf_vao);

    glGenBuffers(1, &hf_vbo_out);
    glBindBuffer(GL_ARRAY_BUFFER, hf_vbo_out);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(LineData), 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(LineData), (void*)sizeof(vec4));

    glBufferData(GL_ARRAY_BUFFER, sizeof(hf_data_in), nullptr, GL_STATIC_READ);  

    //new
    std::vector<vec4> points(FIBER_COUNT);
    std::vector<LineInstance> instances(FIBER_COUNT);

    for (unsigned int i = 0; i < FIBER_COUNT; i++)
    {
        float t = (float)i/(float)FIBER_COUNT;
        points[i] = vec4(spherePath(t));
        instances[i] = {1.0f, FIBER_SIZE,i*FIBER_SIZE,0};
    }

    spherePointsCPU = points;
    spherePoints.uploadData(points);
    lineInstances.uploadData(instances);

    return true; 
}
bool HopfSimulation::initControllerData() 
{
    c_cam = new Camera(vec3({-1,0,0}),vec3({2,0,0}),CONTROL_VIEWPORT_SIZE,CONTROL_VIEWPORT_SIZE,PI/4);
    c_sphere = new Sphere(128,128);
    m_controller = new PointController();

    c_cam->init();
    return true;
}

void HopfSimulation::renderFibers() 
{   
    ShaderProgram hopf_map = shaderManager->getProgram("hopf");
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,0,spherePoints.id());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,1,lineInstances.id());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,2, hf_vbo_out);

    hopf_map.use();
    hopf_map.setUniform("numFibers",(unsigned int)FIBER_COUNT);
    hopf_map.dispatchCompute(FIBER_COUNT, FIBER_SIZE, 1);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

    glBindVertexArray(hf_vao);

    // Render hopf fibration in main viewport
    glViewport(0, 0, m_width, m_height);
    m_camera.setScreenRatio(m_width,m_height);
    m_camera.updateUniformData();
    m_camera.bindUbo(0);

    glDepthMask(GL_TRUE);
    hf_main_shader.use();
    glMultiDrawArrays(GL_LINE_LOOP,hf_draw_firsts,hf_draw_counts,hf_draw_max);

    glBindBufferBase(GL_UNIFORM_BUFFER,0,0);
}

void HopfSimulation::renderControlSphere()
{   
    // Render sphere in bottom right corner
    glViewport(m_width - CONTROL_VIEWPORT_SIZE, 0, CONTROL_VIEWPORT_SIZE, CONTROL_VIEWPORT_SIZE);
    c_cam->updateUniformData();
    c_cam->bindUbo(0);

    c_sphere->render(c_shader);
    m_controller->render(c_shader,spherePointsCPU);   
    glBindBufferBase(GL_UNIFORM_BUFFER,0,0);
}
void HopfSimulation::renderUI()
{
    // Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	// Parameters controls
	ImGui::Begin("Parameters");                          
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
    
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
    	glfwGetFramebufferSize(m_window, &m_width, &m_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //m_controller->transform(mat4(rot_axis<GLfloat>(vec3{0,1,1},PI/256)));

        //Rendering
        renderFibers();
        renderControlSphere();
        renderUI();

    	glfwSwapBuffers(m_window);    
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}