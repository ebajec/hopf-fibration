#ifndef UI_H
#define UI_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "defines.h"
#include "camera.h"
#include "imgui_internal.h"

static inline ivec2 fromImVec2(ImVec2 v) { return ivec2(v.x,v.y);}
static inline ImVec2 fromivec2(ivec2 v) { return ImVec2(v.x,v.y);}

class ImGuiContextGLFW
{
public:
    ImGuiContextGLFW(GLFWwindow * window);
    ~ImGuiContextGLFW();

    ImGuiContext * ptr;
    GLFWwindow * window;
};

class ImGuiElement
{
public:
    virtual bool add(ImGuiContextGLFW * ctx);
    const char * name = "Debug";
protected:
    ImGuiContextGLFW * ctx = nullptr;
    ImGuiID id;
};

class Viewport : public ImGuiElement
{
public:
    Viewport();
    Viewport(int width, int height, ivec2 pos,Camera camera);

    bool add(ImGuiContextGLFW * ctx);
    void render();

    void setRenderCallback(void* usr, void (*renderCallback)(void* usr, Camera& camera));
    void setBehavior(void* usr, void (*behaviorCallback)(void* usr, Viewport* viewport));

    void setSize(int width, int height);
    void fixPos(ivec2 pos);
    void fixSize(ivec2 size);
    void unfixPos() {isFixedPos = false;};
    void unfixSize() {isFixedSize = false;};

    void setCamera(const Camera& camera);
    ivec2 getPos(){return m_pos;}

private:   
    Camera m_camera;

    ivec2 m_pos;
    ivec2 m_size;   

    void (*m_behaviorCallback)(void* usr, Viewport* viewport) = nullptr;
    void (*m_renderCallback)(void* usr, Camera& camera) = nullptr;
   
    bool isCollapsed = false;
    bool isFixedPos = false;
    bool isFixedSize = false;

    void* m_usr = nullptr;
};

#endif