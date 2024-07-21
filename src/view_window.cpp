#include "view_window.h"

BaseViewWindow::BaseViewWindow(
	const char* title, int width, int height, GLFWmonitor* monitor, GLFWwindow* share
) : w_width(width), w_height(height) {

	/************** SET UP KEYBINDS **************/

	auto map_movement = [this](int action) {
		//mval determines whether motion in a direction should start or stop
		int mval = 0;
		switch (action)
		{
			case GLFW_PRESS:   mval =  1; break;
			case GLFW_RELEASE: mval = -1; break;
		}

		vec3& dir = this->w_cam_manager.cam_motion_dir;

		this->w_key_manager.mapKey(GLFW_KEY_W, action, 
			[&dir, mval]() {*dir[2] += +mval; });
		this->w_key_manager.mapKey(GLFW_KEY_A, action, 
			[&dir, mval]() {*dir[0] += -mval; });
		this->w_key_manager.mapKey(GLFW_KEY_S, action, 
			[&dir, mval]() {*dir[2] += -mval; });
		this->w_key_manager.mapKey(GLFW_KEY_D, action, 
			[&dir, mval]() {*dir[0] += +mval; });
		this->w_key_manager.mapKey(GLFW_KEY_LEFT_SHIFT, action, 
			[&dir, mval]() {*dir[1] += -mval; });
		this->w_key_manager.mapKey(GLFW_KEY_SPACE, action, 
			[&dir, mval]() {*dir[1] += mval; });
	};

	map_movement(GLFW_PRESS);
	map_movement(GLFW_RELEASE);

	// Press ESC to enable or disable camera controls
	w_key_manager.mapKey(GLFW_KEY_ESCAPE, GLFW_PRESS, [this]()
	{ 
		if (w_state.control_state == WinState::CONTROL_CAMERA) 
		{
			_disableCameraControls(); 
			w_state.control_state = WinState::CONTROL_GUI;
		}
		else  
		{
			_enableCameraControls(); 
			w_state.control_state = WinState::CONTROL_CAMERA;
		}
	});

	/************** SET UP CAMERA **************/
	w_cam = Camera(
		vec3({ -1,0,0 }),
		vec3({ 10,0,0 }),
		w_width,
		w_height,
		PI / 4);
	w_cam_manager.attach(&w_cam);

	this->launch(title,monitor,share);
}

void BaseViewWindow::launch(const char* title, GLFWmonitor* monitor, GLFWwindow* share)
{
	w_main_thread = std::thread(&BaseViewWindow::_windowProgram, this, title, monitor, share);

	// Do nothing until done initializing
	while(!isRunning()){}

	return;
}

void BaseViewWindow::close()
{
	glfwSetWindowShouldClose(window, true);
	return;
}

void BaseViewWindow::waitForClose()
{
	w_main_thread.join();
	return;
}

void BaseViewWindow::_windowProgram(const char* title, GLFWmonitor* monitor, GLFWwindow* share)
{
	window = glfwCreateWindow(w_width, w_height, title, monitor, share);
	if (!window) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
	}
	glfwMakeContextCurrent(window);
	glfwSetWindowPos(window,0,0);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	glfwSetWindowUserPointer(window, this);
	glfwSetKeyCallback(window, _keyCallback);
	glfwSetCursorPosCallback(window, _cursorPosCallback);
	
	const GLubyte* _renderer = glGetString(GL_RENDERER);
	const GLubyte* _version = glGetString(GL_VERSION);
	printf("Renderer: %s\n", _renderer);
	printf("OpenGL version supported %s\n", _version);

	w_cam.init();
	w_state.is_running = true;
	w_cam_manager.start(&this->w_state);

	_main();

	_disableCameraControls();
	w_cam_manager.stop();
	w_state.is_running = false;
	glfwDestroyWindow(window);
	return;
}

void BaseViewWindow::_keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	auto win = static_cast<BaseViewWindow*>(glfwGetWindowUserPointer(window));

	win->w_key_manager.callKeyFunc(key, action);
}

void BaseViewWindow::_cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	auto win = static_cast<BaseViewWindow*>(glfwGetWindowUserPointer(window));
	int width, height;
    glfwGetWindowSize(window, &width, &height);

	auto dx = xpos - width/2;
	auto dy = ypos - height/2;

	if (win->w_state.control_state == WinState::CONTROL_CAMERA) {
		win->w_cam_manager.rotate(dx,dy);
		glfwSetCursorPos(window,width/2,height/2);
	}	
}

void BaseViewWindow::_enableCameraControls()
{
	//Center cursor so camera does not jerk
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (glfwRawMouseMotionSupported()) {
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}
	glfwSetCursorPos(window,w_width/2,w_height/2);
	w_state.control_state = WinState::CONTROL_CAMERA;
	return;
}

void BaseViewWindow::_disableCameraControls()
{
	w_state.control_state = WinState::CONTROL_GUI;
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	if (glfwRawMouseMotionSupported()) {
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
	}
	return;
}

void KeyManager::callKeyFunc(int key, int action)
{
	pair<int, int> operation({ key,action });

	if (keymap.contains(operation)) keymap.at(operation)();

	return;
}

void CameraManager::attach(Camera* in_cam) {
	this->cam = in_cam;
}

void CameraManager::_update_loop(WinState* state)
{
	while (true) {
		if (!state->is_running) return;
		if (state->control_state == WinState::CONTROL_CAMERA)
			cam->translate(cam_motion_dir * pow(cam_movespeed,2)*1e-4f);
	}
	return;
}

void CameraManager::start(WinState* state)
{
	cam_motion_dir = { 0,0,0 };
	cursor_pos = { 0,0 };
	updater = thread(&CameraManager::_update_loop, this, state);
	return;
}

void CameraManager::stop()
{
	updater.detach();
	cam->reset();
	return;
}

void CameraManager::rotate(double dx, double dy)
{
	dx *= -cam_sensitivity;
	dy *= -cam_sensitivity;
	cam->rotate(dy, dx);
	return;
}

