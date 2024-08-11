#pragma once

#ifndef GLFW_WINDOW_H
#define GLFW_WINDOW_H
#include <GL/glew.h> 
#include <GLFW/glfw3.h>
#include <thread>
#include <functional>
#include <map>
#include "camera.h"

class CamManager;
class KeyManager;
class BaseViewWindow;

struct WinState{
	enum ControlState
	{
		CONTROL_GUI,
		CONTROL_CAMERA	
	};
	bool is_running = false;
	ControlState control_state = CONTROL_GUI;
};

/*Maps or unmaps keys + actions to functions, and holds information regarding the 
* respective function for a key and action pair. 
* 
* Keys and actions must be identified with GLFW macros.  Look at GLFW docs for 
* more info.
*/
class KeyManager {
private:
	std::map<std::pair<int, int>, std::function<void()>> keymap;
public:
	KeyManager() {}
	//maps function to key action
	void mapKey(int key, int action, std::function<void()> func) { keymap.insert({ {key,action},func }); }
	void unmap(int key, int action) { keymap.erase({ key,action }); }
	//calls function for key action, if one exists  
	void callKeyFunc(int key, int action);
};

/*Manages camera operations for BaseViewWindow. Updates to position and 
* orientation are performed on "_updater_thread."
*
* For this to work, attach(Camera*) must be called on a camera instance
* to allow this class to update it. 
*/
class CameraManager {
private:
	std::thread updater;
	Camera* cam;
	vec2 cursor_pos = { 0,0 };

	void updateThread(WinState* state);
public:

	/*Camera will move in this direction each camera update. Camera::translate()
	* is called with motion_dir as the argument.*/
	vec3 cam_motion_dir = { 0,0,0 };
	float cam_movespeed = 0.2;
	float cam_sensitivity = 0.004;

	CameraManager() {}

	//sets Camera object to be controlled
	void attach(Camera* in_cam);
	//launches thread which contiuously updates camera
	void start(WinState* state);
	//terminates updater thread
	void stop();
	//rotates camera based of difference between old and new cursor pos
	void rotate(double x_new, double y_new);

	friend class BaseViewWindow;
};

/*
* Provides a base for creating windows with GLFW.  Natively handles keyboard
* input, mouse input, and camera controls.  By default, moves camera with WASD
* and mouse.  
*/
class BaseViewWindow {

public:
	BaseViewWindow(const char* title, int width, int height, GLFWmonitor* monitor = NULL, GLFWwindow* share = NULL);
	~BaseViewWindow();
	bool isRunning() { return this->m_state.is_running;}
	void launch(const char* title, GLFWmonitor* monitor, GLFWwindow* share );
	void close();
	void waitForClose();

protected:
	//Intializes window, runs main loop until close, then terminates OpenGL context
	void windowProgram(
		const char* title,
		GLFWmonitor* monitor,
		GLFWwindow* share);

	//Main loop of program. 
	virtual void windowLoop() {};

	//callback for keyboard input event
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

	//callback for cursor position update event
	static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
	
	void enableCameraControls();
	void disableCameraControls();

	int m_height;
	int m_width;

	GLFWwindow* m_window = NULL;
	WinState m_state;
	Camera m_camera;
	KeyManager m_keyManager;
	CameraManager m_cameraManager;
};






#endif
