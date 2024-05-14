#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "view_window.h"
#include "shader.h"
#include "matrix.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#define WIDTH 1280                  
#define HEIGHT 720

#define PI 3.141592654f

#define SPH_ATTR_COUNT 3            

#define CONTROL_VIEWPORT_SIZE 600   // Determines height/width of lower-right viewport

#define FIBER_COUNT 100             // Number of fibers to compute
#define FIBER_SIZE 1000             // Number of samples in each fiber

#define HF_CAMERA_BINDING 1
#define C_CAMERA_BINDING 0

/**********************************************************************************
 * 
 * Helper functions
 * 
 ***********************************************************************************/
vec3 S2(GLfloat phi, GLfloat theta) 
{
    return vec3{sin(theta)*cos(phi),sin(theta)*sin(phi),cos(theta)};
}
void printData(GLfloat* data, int x, int y) 
{
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            std::cout << data[i*y + j] << " ";
        }
        std::cout << "\n";
    }
}
template<typename T> std::vector<T> lerp(T start, T end, int steps) 
{
    std::vector<T>  out(steps+1);   
    T diff = end - start;
    float step = pow(steps,-1);

    for (int i = 0; i < steps + 1; i++) {
        out[i] = start + (i*step)*diff;
    }
    return out;
}
unsigned int factorial(int n) {
    return (n > 0) ? n*factorial(n-1) : 1;
}
template<int n,typename T> matrix<n,n,T> exp(matrix<n,n,T> mat) {
    auto result = matrix<n,n,T>::id();
    result = result + mat;
    mat = mat*mat;
    result = result + T(0.5)*mat;
    mat = mat*mat;
    result = result + T(pow(6.0f,-1))*mat;
    mat = mat*mat;
    result = result + T(pow(24.0f,-1))*mat;
    return result;
}
float uRand() 
{
    return static_cast<float>(rand())/static_cast<float>(RAND_MAX);

}
/**********************************************************************************
 *
 * Sphere
 * 
 **********************************************************************************/
template<int phi_step, int theta_step> class Sphere {
public:
    Sphere() {}
    ~Sphere();
    void init(vec3 color = {0.5,0.5,0.5},GLuint usage = GL_STATIC_DRAW);
    void setScale(GLfloat s);
    void setPos(vec3 pos);
    void rotate(vec3 axis, GLfloat angle);
    void render(ShaderProgram& shader);
    
    enum ATTRIBUTES {POSITION,COLOR,NORMAL};
private:
    
    GLuint vao;             
    GLuint vbo[SPH_ATTR_COUNT];                   
    GLint draw_firsts[phi_step];        // For glMultiDrawArrays
    GLsizei draw_counts[theta_step];    // For glMultiDrawArrays

    GLfloat scale = 1;
    mat4 geometry = mat4::id();
};
/**********************************************************************************
 * 
 * Implementation details for Sphere.
 * 
************************************************************************************/
template<int phi_step, int theta_step> Sphere<phi_step,theta_step>::~Sphere() {
    glDeleteBuffers(3,vbo);
    glDeleteVertexArrays(1,&vao);
}
template<int phi_step, int theta_step> void Sphere<phi_step,theta_step>::init(vec3 color,GLuint usage) {
    int data_size = 8*phi_step*theta_step;
    GLfloat* data_pos = new GLfloat[data_size];
    GLfloat* data_color = new GLfloat[data_size];

    vector<GLfloat> phi = lerp(static_cast<GLfloat>(0),2*PI,phi_step);
    vector<GLfloat> theta = lerp(static_cast<GLfloat>(0),PI,theta_step);

    int idx = 0;
    vec4 p;
    auto s_color = [](GLfloat in_phi, GLfloat in_theta) {
        vec3 s = S2(in_phi/4,in_theta/2);
        return vec4{pow(*s[0],2),pow(*s[1],2),pow(*s[2],2),1};
    };   

    for (int i = 0; i < phi_step; i++) {
        for (int j = 0; j < theta_step; j++) {
            // First point
            p = vec4(S2(phi[i],theta[j]));
            memcpy(data_pos + idx, p.data(),4*sizeof(float));

            p = vec4(color);//s_color(phi[i],theta[j]);
            memcpy(data_color + idx, p.data(),4*sizeof(float));

            idx += 4;

            // Second point
            p = vec4(S2(phi[i+1],theta[j+1]));
            memcpy(data_pos + idx, p.data(),4*sizeof(float));

            p = vec4(color);//s_color(phi[i+1],theta[j+1]);
            memcpy(data_color + idx, p.data(),4*sizeof(float));

            idx += 4;
        }
    }

    // Need to set this up to use glMultiDrawArrays
    for (int i = 0; i < phi_step; i++){ 
        draw_firsts[i] = 2*theta_step*i;
        draw_counts[i] = 2*theta_step;
    }

    glGenVertexArrays(1,&vao);
    glBindVertexArray(vao);
    for (int i = 0; i < SPH_ATTR_COUNT; i++) {
        glEnableVertexAttribArray(i);
        glGenBuffers(1,vbo + i);
    }

    glBindBuffer(GL_ARRAY_BUFFER,vbo[POSITION]);
    glBufferData(GL_ARRAY_BUFFER,data_size*sizeof(float),data_pos,usage);
    glVertexAttribPointer(POSITION, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER,vbo[COLOR]);
    glBufferData(GL_ARRAY_BUFFER,data_size*sizeof(float),data_color,usage);
    glVertexAttribPointer(COLOR, 4, GL_FLOAT, GL_FALSE, 0, 0);

    // On the unit sphere we can just use the positions as normals!
    glBindBuffer(GL_ARRAY_BUFFER,vbo[NORMAL]);
    glBufferData(GL_ARRAY_BUFFER,data_size*sizeof(float),data_pos,usage);
    glVertexAttribPointer(NORMAL, 4, GL_FLOAT, GL_FALSE, 0, 0);

    delete[] data_pos;
    delete[] data_color;
    return;
}
template<int phi_step, int theta_step> void Sphere<phi_step,theta_step>::render(ShaderProgram& shader) {
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

    shader.use();
    shader.setUniform("obj_geometry",geometry,GL_TRUE);
    shader.setUniform("scale",scale);
    glBindVertexArray(vao);
    //glDrawArrays(GL_TRIANGLE_STRIP,0,2*phi_step*theta_step);
    glMultiDrawArrays(GL_TRIANGLE_STRIP,draw_firsts,draw_counts,phi_step);
    return;
}
template<int phi_step, int theta_step> void Sphere<phi_step,theta_step>::setScale(GLfloat in_scale) 
{
    this->scale = in_scale;
    return;
}
template<int phi_step, int theta_step> void Sphere<phi_step,theta_step>::setPos(vec3 pos) 
{
    geometry[0][3] = pos[0][0];
    geometry[1][3] = pos[1][0];
    geometry[2][3] = pos[2][0];
    return;
}
template <int phi_step, int theta_step> void Sphere<phi_step,theta_step>::rotate(vec3 axis, GLfloat angle) 
{   
    axis = normalize(axis);
    mat3 differential = mat3{
         0,         -*axis[2],   *axis[1],
         *axis[2],   0,         -*axis[0],
        -*axis[1],   *axis[0],   0
    };
    mat4 rotation = mat4(exp(angle*differential));
    geometry = geometry * rotation;
    return;
} 

/**********************************************************************************
 * 
 * Control sphere manager
 * 
 **********************************************************************************/
#define SPHERE_SIZE 128
#define SCOUNT 100
class SphereController{
public:
    SphereController() {}
    void init();
    void render(ShaderProgram& shader);
    void transform(mat4 trans);
private:
    mat4 geometry = mat4::id();
    vector<vec3> points;
    Sphere<SPHERE_SIZE,SPHERE_SIZE> sphere;

};
/**********************************************************************************
 * 
 * Implementation details for SphereController
 * 
 **********************************************************************************/
void SphereController::init() 
{
    points = vector<vec3>(SCOUNT);
    for (int i = 0; i < SCOUNT; i++) 
        points[i] = S2(uRand()*2*PI,uRand()*PI);
    sphere.init(vec3{1,0,0});
    sphere.setScale(0.07f);
    return;
}
void SphereController::render(ShaderProgram& shader)
{   
    for (int i = 0; i < SCOUNT; i++) 
    {
        sphere.setPos(vec3(geometry*vec4(points[i])));   
        sphere.render(shader);
    }
}
void SphereController::transform(mat4 trans) 
{
    geometry = geometry*trans;
}

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
    HopfSimulation(int width, int height);
    ~HopfSimulation();

protected:
    void _main();
    bool initFiberData();
    bool initControllerData();
    bool initShaders();
    void renderFibers();
    void renderControlSphere();
    void renderUI();

    SphereController controller;
    ShaderProgram c_shader;
    Camera c_cam;
    Sphere<128,128> c_sphere;
    
    ShaderProgram hf_main_shader;                   // Handles rendering of main scene 
    ShaderProgram hf_map;                           // Computes fibers of hopf map
    GLuint hf_vao;                               
    GLuint hf_vbo_in; 
    GLuint hf_vbo_out; 
    GLfloat hf_data_in[4*FIBER_COUNT*FIBER_SIZE];   // Arguments to hf_map; allows parallel computation of fibers.
    GLint hf_draw_firsts[FIBER_COUNT];              // For glMultiDrawArrays
    GLsizei hf_draw_counts[FIBER_COUNT];            // For glMultiDrawArrays
    float hf_anim_speed;                         
    int hf_draw_max = FIBER_COUNT;                  // Parameter to control number of drawn circles
};

/**********************************************************************************
 * 
 * Implementation details for HopfSimulation.
 * 
 **********************************************************************************/
HopfSimulation::HopfSimulation(int width, int height) : BaseViewWindow(width, height) {
    c_cam = Camera(vec3({-1,0,0}),vec3({2,0,0}),CONTROL_VIEWPORT_SIZE,CONTROL_VIEWPORT_SIZE,PI/4);
}
HopfSimulation::~HopfSimulation() {
    glDeleteBuffers(1,&hf_vbo_in);
    glDeleteBuffers(1,&hf_vbo_out);
    glDeleteVertexArrays(1,&hf_vao);
}
bool HopfSimulation::initShaders() {

    // Initialize main shader and camera

    if(!hf_main_shader.addShader("../shader/main_vertex.glsl",GL_VERTEX_SHADER)||
       !hf_main_shader.addShader("../shader/main_frag.glsl",GL_FRAGMENT_SHADER))
        return false;
    hf_main_shader.link();

    if (!w_cam.bindToShader(hf_main_shader,"Camera",HF_CAMERA_BINDING)) return false;
    
    
    // Initialize hopf map shader and fiber data

    if(!hf_map.addShader("../shader/hopf_map.glsl",GL_VERTEX_SHADER))
        return false;
    const GLchar* hf_feedback_varyings[] = { "data_out" };
    glTransformFeedbackVaryings(hf_map.program, 1, hf_feedback_varyings, GL_INTERLEAVED_ATTRIBS);
    hf_map.link();

    // Initialize control window shader and camera

    if (!c_shader.addShader("../shader/control_window_vertex.glsl",GL_VERTEX_SHADER)||
        !c_shader.addShader("../shader/control_window_frag.glsl",GL_FRAGMENT_SHADER))
        return false;
    c_shader.link();

    if (!c_cam.bindToShader(c_shader,"Camera",C_CAMERA_BINDING)) return false;
    
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
    c_cam.init();
    c_sphere.init();
    controller.init();

    return true;
}
void HopfSimulation::renderFibers() 
{
    hf_map.use();
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
    hf_main_shader.use();
    glMultiDrawArrays(GL_LINE_LOOP,hf_draw_firsts,hf_draw_counts,hf_draw_max);

}
void HopfSimulation::renderControlSphere()
{   
    // Render sphere in bottom right corner
    glViewport(w_width - CONTROL_VIEWPORT_SIZE, 0, CONTROL_VIEWPORT_SIZE, CONTROL_VIEWPORT_SIZE);
    c_cam.updateUniformData();

    c_sphere.render(c_shader);
    controller.render(c_shader);   
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

        controller.transform(mat4(rot_axis<GLfloat>(vec3{0,1,1},PI/256)));

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

int main()
{
    if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
	}

    HopfSimulation sim(WIDTH,HEIGHT);
    
    sim.launch("Hopf Fibration",NULL,NULL);
     
    printf("-----------------------------\n");
    printf("Press ESC to toggle GUI access\n");
    printf("Use WASD and mouse to move\n");

    sim.waitForClose();
    glfwTerminate();
    return 0;
}
