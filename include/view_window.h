#pragma once

#ifndef VIEW_WINDOW_H
#define VIEW_WINDOW_H
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
	bool is_running = false;
	bool mouse_enabled = false;
};

/*Maps or unmaps keys + actions to functions, and holds information regarding the 
* respective function for a key and action pair. 
* 
* Keys and actions must be identified with GLFW macros.  Look at GLFW docs for 
* more info.
*/
class KeyManager {
private:
	map<pair<int, int>, std::function<void()>> keymap;
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
	thread _updater;
	Camera* _cam = nullptr;
	vec2 _cursor_pos = { 0,0 };

	void _update_loop(WinState* state);
public:

	/*Camera will move in this direction each camera update. Camera::translate()
	* is called with motion_dir as the argument.*/
	vec3 motion_dir = { 0,0,0 };
	float movespeed = 0.2;
	float sensitivity = 0.004;

	CameraManager() {}

	//sets Camera object to be controlled
	void attach(Camera* cam) { _cam = cam; }
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
* 
* IMPORTANT: The pure virtual function "_main()" is meant to contain the body 
* of the program running in the window.  Some initialization is done before this
* is called.  
*
*/
class BaseViewWindow {
protected:
	GLFWwindow* _window = NULL;

	int _height;
	int _width;
	WinState state;

	Camera _cam;
	KeyManager _key_manager;
	CameraManager _cam_manager;
	bool _initialized = false;

	
	//Intializes window, runs main loop until close, then terminates OpenGL context
	void _windowProgram(
		const char* title,
		GLFWmonitor* monitor,
		GLFWwindow* share);

	//Main loop of program. 
	virtual void _main() = 0;

	//callback for keyboard input event
	static void _keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	//callback for cursor position update event
	static void _cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
	void _enableMouseControls();
	void _disableMouseControls();

public:
	thread main_thread;
	BaseViewWindow(
		int width,
		int height
	);

	bool isRunning() { return this->state.is_running;}

	void launch(
		const char* title,
		GLFWmonitor* monitor,
		GLFWwindow* share
	);

	void close();
};






#endif
