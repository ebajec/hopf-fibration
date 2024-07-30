#ifndef UI_H
#define UI_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

class ImguiContextGLFW
{
public:
    ImguiContextGLFW(GLFWwindow * window);
    ~ImguiContextGLFW();

private:
    GLFWwindow * m_window;
};

#endif