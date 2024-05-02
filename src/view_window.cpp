#include "view_window.h"

BaseViewWindow::BaseViewWindow(
	int width,
	int height
) : _width(width), _height(height) {

	/************** SET UP KEYBINDS **************/

	//map_keys is to condense things
	auto map_movement = [this](int action) {
		//mval determines whether motion in a direction should start or stop
		int mval = (action == GLFW_PRESS) - (action == GLFW_RELEASE);
		vec3& dir = this->_cam_manager.motion_dir;
		this->_key_manager.mapKey(GLFW_KEY_W, action, [&dir, mval]() {*dir[2] += +mval; });
		this->_key_manager.mapKey(GLFW_KEY_A, action, [&dir, mval]() {*dir[0] += -mval; });
		this->_key_manager.mapKey(GLFW_KEY_S, action, [&dir, mval]() {*dir[2] += -mval; });
		this->_key_manager.mapKey(GLFW_KEY_D, action, [&dir, mval]() {*dir[0] += +mval; });
		this->_key_manager.mapKey(GLFW_KEY_LEFT_SHIFT, action, [&dir, mval]() {*dir[1] += -mval; });
		this->_key_manager.mapKey(GLFW_KEY_SPACE, action, [&dir, mval]() {*dir[1] += mval; });};
	map_movement(GLFW_PRESS);
	map_movement(GLFW_RELEASE);

	// Press ESC to enable or disable camera controls
	this->_key_manager.mapKey(GLFW_KEY_ESCAPE, GLFW_PRESS, [this]()
	{ 
		if (this->state.mouse_enabled) _disableMouseControls(); else _enableMouseControls(); 
	});

	/************** SET UP CAMERA **************/
	_cam = Camera(
		vec3({ -1,0,0 }),
		vec3({ 10,0,0 }),
		_width,
		_height,
		PI / 4);
	_cam_manager.attach(&_cam);
}

void BaseViewWindow::launch(const char* title, GLFWmonitor* monitor, GLFWwindow* share)
{
	main_thread = std::thread(&BaseViewWindow::_windowProgram, this, title, monitor, share);
}

void BaseViewWindow::close()
{
	glfwSetWindowShouldClose(_window, true);
	return;
}

void BaseViewWindow::_windowProgram(const char* title, GLFWmonitor* monitor, GLFWwindow* share)
{
	_window = glfwCreateWindow(_width, _height, title, monitor, share);
	if (!_window) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
	}
	glfwMakeContextCurrent(_window);
	glfwSetWindowPos(_window,0,0);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	glfwSetWindowUserPointer(_window, this);
	glfwSetKeyCallback(_window, _keyCallback);
	glfwSetCursorPosCallback(_window, _cursorPosCallback);
	
	const GLubyte* _renderer = glGetString(GL_RENDERER);
	const GLubyte* _version = glGetString(GL_VERSION);
	printf("Renderer: %s\n", _renderer);
	printf("OpenGL version supported %s\n", _version);

	_cam.init();
	state.is_running = true;
	_cam_manager.start(&this->state);

	_main();

	_disableMouseControls();
	_cam_manager.stop();
	state.is_running = false;
	glfwDestroyWindow(_window);
	return;
}

void BaseViewWindow::_keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	auto win = static_cast<BaseViewWindow*>(glfwGetWindowUserPointer(window));

	win->_key_manager.callKeyFunc(key, action);
}

void BaseViewWindow::_cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	auto win = static_cast<BaseViewWindow*>(glfwGetWindowUserPointer(window));
	int width, height;
    glfwGetWindowSize(window, &width, &height);

	auto dx = xpos - width/2;
	auto dy = ypos - height/2;

	if (win->state.mouse_enabled) {
		win->_cam_manager.rotate(dx,dy);
		glfwSetCursorPos(window,width/2,height/2);
	}	
}

void BaseViewWindow::_enableMouseControls()
{
	//Center cursor so camera does not jerk
	glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (glfwRawMouseMotionSupported()) {
		glfwSetInputMode(_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}
	glfwSetCursorPos(_window,_width/2,_height/2);
	state.mouse_enabled = true;
	return;
}

void BaseViewWindow::_disableMouseControls()
{
	state.mouse_enabled = false;
	glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	if (glfwRawMouseMotionSupported()) {
		glfwSetInputMode(_window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
	}
	return;
}

void KeyManager::callKeyFunc(int key, int action)
{
	pair<int, int> keyaction({ key,action });

	if (keymap.contains(keyaction)) keymap.at(keyaction)();

	return;
}

void CameraManager::_update_loop(WinState* state)
{
	while (true) {
		if (!state->is_running) return;
		if (state->mouse_enabled)
			_cam->translate(motion_dir * pow(movespeed,2)*1e-4f);
	}
	return;
}

void CameraManager::start(WinState* state)
{
	motion_dir = { 0,0,0 };
	_cursor_pos = { 0,0 };
	_updater = thread(&CameraManager::_update_loop, this, state);
	return;
}

void CameraManager::stop()
{
	_updater.detach();
	_cam->reset();
	return;
}

void CameraManager::rotate(double dx, double dy)
{
	dx *= -sensitivity;
	dy *= -sensitivity;
	_cam->rotate(dy, dx);
	return;
}

