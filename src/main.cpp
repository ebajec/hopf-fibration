#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "view_window.h"
#include "shader.h"
#include "matrix.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#define WIDTH 800
#define HEIGHT 800
#define PI 3.141592654f
#define SPH_ATTR_COUNT 3
#define PATH_SIZE 100
#define CIRC_SIZE 1000

vec3 S2(GLfloat phi, GLfloat theta) {
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
template<typename T> 
std::vector<T> lerp(T start, T end, int steps) {
    std::vector<T>  out(steps+1);   
    T diff = end - start;
    float step = pow(steps,-1);

    for (int i = 0; i < steps + 1; i++) {
        out[i] = start + (i*step)*diff;
    }
    return out;
}

/**********************************************************************************
 *
 * Sphere.
 * 
 **********************************************************************************/
template<int phi_step, int theta_step> class Sphere {
public:
    Sphere() {}
    void init(GLuint usage = GL_STATIC_DRAW);
    //void setScale(GLfloat s);
    //void setPos(vec3 pos);
    void render(ShaderProgram shader);
    
    enum ATTR {POSITION,COLOR,NORMAL};
private:
    mat4 transformation = mat4::id();   // Affine transformation applied to object.
    GLuint vao;             
    GLuint vbo[SPH_ATTR_COUNT];                      
    GLint draw_firsts[phi_step];        // For glMultiDrawArrays
    GLsizei draw_counts[theta_step];    // For glMultiDrawArrays
};
template<int phi_step, int theta_step> void Sphere<phi_step,theta_step>::init(GLuint usage) {
    int data_size = 8*phi_step*theta_step;
    GLfloat* data_pos = new GLfloat[data_size];
    GLfloat* data_color = new GLfloat[data_size];

    vector<GLfloat> phi = lerp(static_cast<GLfloat>(0),2*PI,phi_step);
    vector<GLfloat> theta = lerp(static_cast<GLfloat>(0),PI,theta_step);

    int idx = 0;
    vec4 p;

    for (int i = 0; i < phi_step; i++) {
        for (int j = 0; j < theta_step; j++) {
            // First point
            p = vec4(S2(phi[i],theta[j]));
            memcpy(data_pos + idx, p.data(),4*sizeof(float));

            p = vec4{1,1,1,1};
            memcpy(data_color + idx, p.data(),4*sizeof(float));

            idx += 4;

            // Second point
            p = vec4(S2(phi[i + 1],theta[j+1]));
            memcpy(data_pos + idx, p.data(),4*sizeof(float));

            p = vec4{1,1,1,1};
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
template<int phi_step, int theta_step> void Sphere<phi_step,theta_step>::render(ShaderProgram shader) {
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

    shader.use();
    glBindVertexArray(vao);
    //glDrawArrays(GL_TRIANGLE_STRIP,0,2*phi_step*theta_step);
    glMultiDrawArrays(GL_TRIANGLE_STRIP,draw_firsts,draw_counts,phi_step);
    return;
}


/**********************************************************************************
 * 
 * Main window/program.  Renders hopf fibration and includes UI to control
 * some parameters.
 * 
 **********************************************************************************/
class HopfSimulation : public BaseViewWindow {
public:
    HopfSimulation(int width, int height);
    ~HopfSimulation();

protected:
    void _main();
    void _renderFibers();
    void _initFiberData();
    void _renderUI();
    
    ShaderProgram hf_main_shader;               // Handles rendering of main scene 
    ShaderProgram hf_map;                       // Computes fibers of hopf map
    GLuint hf_vao;                              
    GLuint hf_vbo_in;
    GLuint hf_vbo_out;
    GLfloat hf_data_in[4*PATH_SIZE*CIRC_SIZE];  // Arguments to hf_map; allows parallel computation of fibers.
    GLint hf_draw_firsts[PATH_SIZE];            // For glMultiDrawArrays
    GLsizei hf_draw_counts[PATH_SIZE];          // For glMultiDrawArrays
    float hf_anim_speed;                        
    int hf_draw_max = PATH_SIZE;                // Parameter to control number of drawn circles
};

/**********************************************************************************
 * 
 * Implementation details for HopfSimulation.
 * 
 **********************************************************************************/
HopfSimulation::HopfSimulation(int width, int height) : BaseViewWindow(width, height) {}
HopfSimulation::~HopfSimulation() {
    glDeleteBuffers(1,&hf_vbo_in);
    glDeleteBuffers(1,&hf_vbo_out);
    glDeleteVertexArrays(1,&hf_vao);
}
void HopfSimulation::_renderFibers() 
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

    glDrawArrays(GL_POINTS, 0, PATH_SIZE*CIRC_SIZE);

    glEndTransformFeedback();

    glFlush();
    glDisable(GL_RASTERIZER_DISCARD);

    // Bind output to location 0 and draw fibers
    glBindBuffer(GL_ARRAY_BUFFER,hf_vbo_out);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glDepthMask(GL_TRUE);
    hf_main_shader.use();
    glMultiDrawArrays(GL_LINE_LOOP,hf_draw_firsts,hf_draw_counts,hf_draw_max);

}
void HopfSimulation::_initFiberData() 
{
    // Set up linearly spaced inputs for fiber calculation.
    for (int i = 0; i < PATH_SIZE; i++) {
        hf_draw_firsts[i] = i*CIRC_SIZE;
        hf_draw_counts[i] = CIRC_SIZE;

        for (int j = 0; j < CIRC_SIZE; j++) {
            hf_data_in[4*(CIRC_SIZE*i + j)] = (float)i/(float)PATH_SIZE;
            hf_data_in[4*(CIRC_SIZE*i + j) + 3] = (float)j/(float)CIRC_SIZE;
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
}
void HopfSimulation::_renderUI()
{
    // Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	// Parameters controls
	ImGui::Begin("Parameters");                          
	ImGui::SliderFloat("Animation speed", &this->hf_anim_speed, 0.0f, 1.0f);
	ImGui::SliderInt("Circle count", &this->hf_draw_max, 0, PATH_SIZE);
	ImGui::End();
	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());	
}
void HopfSimulation::_main()
{
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplGlfw_InitForOpenGL(_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Initialize main shader
    hf_main_shader.addShader("../shader/vertex_main.glsl",GL_VERTEX_SHADER);
    hf_main_shader.addShader("../shader/frag_main.glsl",GL_FRAGMENT_SHADER);
    hf_main_shader.link();

    if (!_cam.bindToShader(hf_main_shader,"Camera",0)) return;
    

    // Initialize hopf map shader and fiber data
    hf_map.addShader("../shader/hopf_map.glsl",GL_VERTEX_SHADER);
    const GLchar* feedbackVaryings[] = { "data_out" };
    glTransformFeedbackVaryings(hf_map.program, 1, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);
    hf_map.link();
    _initFiberData();

    // Initialize control ball
    Sphere<30,30> sphere;
    sphere.init();

    while (!glfwWindowShouldClose(_window)) {
        glfwPollEvents();

        // Correct window scaling
    	glfwGetFramebufferSize(_window, &_width, &_height);
    	glViewport(0, 0, _width, _height);

        // Clear buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Initialize camera
		_cam.setScreenRatio(_width,_height);
        _cam.updateUniformData();
        
        //Rendering
        sphere.render(hf_main_shader);
        _renderFibers();
        _renderUI();
    	glfwSwapBuffers(_window);    
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
    sim.main_thread.join();
    glfwTerminate();
    return 0;
}
