

#include "simulation.h"
#include "misc.h"
#include "defines.h"

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

void PointController::render(Shader& shader)
{   
    for (int i = 0; i < SCOUNT; i++) 
    {
        sphere->setPos(vec3(geometry*vec4(points[i])));   
        sphere->render(shader);
    }
}
void PointController::transform(mat4 trans) 
{
    geometry = geometry*trans;
}


/**********************************************************************************
 * 
 * Implementation details for HopfSimulation.
 * 
 **********************************************************************************/
HopfSimulation::HopfSimulation(const char* title, int width, int height) : BaseViewWindow(title, width, height,NULL,NULL) 
{
    _main();
}
HopfSimulation::~HopfSimulation() {
    glDeleteBuffers(1,&hf_vbo_in);
    glDeleteBuffers(1,&hf_vbo_out);
    glDeleteVertexArrays(1,&hf_vao);

    delete c_cam;
    delete c_sphere;
    delete m_controller;
}
bool HopfSimulation::initShaders() {

    // Initialize main shader and camera

    if(!hf_main_shader.addShader("../shader/default_camera.vert",GL_VERTEX_SHADER)||
       !hf_main_shader.addShader("../shader/default_color.frag",GL_FRAGMENT_SHADER))
        return false;
    hf_main_shader.linkProgram();

    if (!w_cam.bindToShader(hf_main_shader,"Camera",HF_CAMERA_BINDING)) return false;

    // Initialize hopf map shader and fiber data

    if(!hf_map.addShader("../shader/hopf_map.glsl",GL_VERTEX_SHADER))
        return false;
    const GLchar* hf_feedback_varyings[] = { "data_out" };
    glTransformFeedbackVaryings(hf_map.program, 1, hf_feedback_varyings, GL_INTERLEAVED_ATTRIBS);
    hf_map.linkProgram();

    // Initialize control window shader and camera

    if (!c_shader.addShader("../shader/control_viewport.vert",GL_VERTEX_SHADER)||
        !c_shader.addShader("../shader/control_viewport.frag",GL_FRAGMENT_SHADER))
        return false;
    c_shader.linkProgram();

    if (!c_cam->bindToShader(c_shader,"Camera",C_CAMERA_BINDING)) return false;
    
    return true;
}   
bool HopfSimulation::initFiberData() 
{
    // Set up linearly spaced inputs for fiber calculation.
    for (int i = 0; i < FIBER_COUNT; i++) {
        hf_draw_firsts[i] = i*FIBER_SIZE;
        hf_draw_counts[i] = FIBER_SIZE;

        for (int j = 0; j < FIBER_SIZE; j++) {
            hf_data_in[4*(FIBER_SIZE*i + j)] = (float)i/(float)FIBER_COUNT;
            hf_data_in[4*(FIBER_SIZE*i + j) + 3] = (float)j/(float)FIBER_SIZE;
        }     
    }
    // Prepare buffers for computing fibers of a hopf map
    glGenVertexArrays(1, &hf_vao);
    glBindVertexArray(hf_vao);
    glEnableVertexAttribArray(0);
    
    glGenBuffers(1, &hf_vbo_in);
    glBindBuffer(GL_ARRAY_BUFFER, hf_vbo_in);
    glBufferData(GL_ARRAY_BUFFER, sizeof(hf_data_in), hf_data_in, GL_STREAM_DRAW);

    glGenBuffers(1, &hf_vbo_out);
    glBindBuffer(GL_ARRAY_BUFFER, hf_vbo_out);
    glBufferData(GL_ARRAY_BUFFER, sizeof(hf_data_in), nullptr, GL_STATIC_READ);  
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
    hf_map.useProgram();
    hf_map.setUniform("ANIM_SPEED",hf_anim_speed);
    hf_map.setUniform("TIME_S",(float)glfwGetTime());

    glBindVertexArray(hf_vao);

    // Bind input data to location 0
    glBindBuffer(GL_ARRAY_BUFFER, hf_vbo_in);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glEnable(GL_RASTERIZER_DISCARD);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, hf_vbo_out);

    glBeginTransformFeedback(GL_POINTS);

    glDrawArrays(GL_POINTS, 0, FIBER_COUNT*FIBER_SIZE);

    glEndTransformFeedback();

    glFlush();
    glDisable(GL_RASTERIZER_DISCARD);

    // Bind output to location 0 and draw fibers
    glBindBuffer(GL_ARRAY_BUFFER,hf_vbo_out);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

    // Render hopf fibration in main viewport
    glViewport(0, 0, w_width, w_height);
    w_cam.setScreenRatio(w_width,w_height);
    w_cam.updateUniformData();

    glDepthMask(GL_TRUE);
    hf_main_shader.useProgram();
    glMultiDrawArrays(GL_LINE_LOOP,hf_draw_firsts,hf_draw_counts,hf_draw_max);

}
void HopfSimulation::renderControlSphere()
{   
    // Render sphere in bottom right corner
    glViewport(w_width - CONTROL_VIEWPORT_SIZE, 0, CONTROL_VIEWPORT_SIZE, CONTROL_VIEWPORT_SIZE);
    c_cam->updateUniformData();

    c_sphere->render(c_shader);
    m_controller->render(c_shader);   
}
void HopfSimulation::renderUI()
{
    // Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	// Parameters controls
	ImGui::Begin("Parameters");                          
	ImGui::SliderFloat("Animation speed", &this->hf_anim_speed, 0.0f, 1.0f);
	ImGui::SliderInt("Circle count", &this->hf_draw_max, 0, FIBER_COUNT);
	ImGui::End();
	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());	
}
void HopfSimulation::_main()
{
    printf("-----------------------------\n");
    printf("Press ESC to toggle GUI access\n");
    printf("Use WASD and mouse to move\n");

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"

    initFiberData();
    initControllerData();
    
    if (!initShaders()) {
        printf("ERROR: Failed to initialize shaders.\n");
        return;
    }
    
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    	glfwGetFramebufferSize(window, &w_width, &w_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_controller->transform(mat4(rot_axis<GLfloat>(vec3{0,1,1},PI/256)));

        //Rendering
        renderFibers();
        renderControlSphere();
        renderUI();

    	glfwSwapBuffers(window);    
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}