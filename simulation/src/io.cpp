#include "io.h"
#include <GLFW/glfw3.h>
#include "imgui.h"

IOManager::IOManager(GLFWwindow * window) : m_window(window)
{
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(m_window, keyCallback);
	glfwSetCursorPosCallback(m_window, cursorPosCallback);
}

void IOManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    ImGuiIO& io = ImGui::GetIO();
    if (action == GLFW_PRESS)
        io.KeysDown[key] = true;
    else if (action == GLFW_RELEASE)
        io.KeysDown[key] = false;

    // Update modifier keys
    io.KeyCtrl = (mods & GLFW_MOD_CONTROL) != 0;
    io.KeyShift = (mods & GLFW_MOD_SHIFT) != 0;
    io.KeyAlt = (mods & GLFW_MOD_ALT) != 0;
    io.KeySuper = (mods & GLFW_MOD_SUPER) != 0;
}

void IOManager::cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{

}