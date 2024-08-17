#include "ui.h"
#include "defines.h"
#include "imgui.h"
#include <GLFW/glfw3.h>

ImGuiContextGLFW::ImGuiContextGLFW(GLFWwindow * window)
{
    this->window = window;
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    this->ptr = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplGlfw_InitForOpenGL(this->window, true);
    ImGui_ImplOpenGL3_Init("#version 430");
}

ImGuiContextGLFW::~ImGuiContextGLFW()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

bool ImGuiElement::add(ImGuiContextGLFW* ctx)
{
    if (!ctx) return false;
    this->id = ImGui::GetID(name);
    this->ctx = ctx;
    return true;
} 

Viewport::Viewport() :
    m_pos(0,0),
    m_size(100,100),
    m_camera(vec3(1,0,0),vec3(0,0,0)),
    m_renderCallback(nullptr),
    m_behaviorCallback(nullptr)
{

}

Viewport::Viewport(int width, int height, ivec2 pos, Camera camera) :
    m_pos(pos),
    m_size(ivec2(width,height)),
    m_camera(camera),
    m_renderCallback(nullptr),
    m_behaviorCallback(nullptr)
{
    m_camera.resize(m_size.x, m_size.y);
}

struct ScreenArea
{
    int x,y,width,height;
};

static ScreenArea convertToGLScreenCoords(ivec2 pos, ivec2 size, int winHeight, int winWidth)
{
    return {pos.x,winHeight-size.y -pos.y,size.x, size.y - 20};
}

bool Viewport::add(ImGuiContextGLFW * ctx)
{
    if (!ImGuiElement::add(ctx)) return false;

    if (isFixedPos) ImGui::SetNextWindowPos(ImVec2(m_pos.x,m_pos.y));
    if (isFixedSize) ImGui::SetNextWindowSize(ImVec2(m_size.x,m_size.y));

    int flags = 
    (isFixedPos  ? ImGuiWindowFlags_NoMove : 0) | 
    (isFixedSize ? ImGuiWindowFlags_NoResize : 0) | ImGuiWindowFlags_NoCollapse
    ;
    
    if (ImGui::Begin(name, nullptr,flags))
    {
        m_pos = fromImVec2(ImGui::GetWindowPos());
        m_size = fromImVec2(ImGui::GetWindowSize());
        isCollapsed = false;

        if (m_behaviorCallback) m_behaviorCallback(m_usr,this);
    }     
    else 
    {
        isCollapsed = true;
    }                  
	ImGui::End();

    return true;
}

void Viewport::render()
{
    int winWidth, winHeight;
    glfwGetWindowSize(ctx->window, &winWidth, &winHeight);

    m_camera.resize(m_size.x, m_size.y);
    m_camera.updateUbo();

    if (!isCollapsed && m_renderCallback)
    {
        ScreenArea glCoords = convertToGLScreenCoords(m_pos, m_size, winHeight, winWidth);
        glViewport(glCoords.x,glCoords.y,glCoords.width,glCoords.height);
        m_renderCallback(m_usr, m_camera);
    }       
}

void Viewport::setRenderCallback(void* usr, void (*renderCallback)(void* usr, Camera& camera))
{
    m_usr = usr;
    m_renderCallback = renderCallback;
}

void Viewport::fixPos(ivec2 pos)
{
    m_pos = pos;
    isFixedPos = true;
}

void Viewport::fixSize(ivec2 size)
{
    m_size = size;
    isFixedSize = true;
}

