#ifndef IO_H
#define IO_H


#include <GLFW/glfw3.h>

class IOManager
{
public:
    IOManager(GLFWwindow * window);

	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

private:
    GLFWwindow * m_window;
};

#endif