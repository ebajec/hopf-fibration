#include "window.h"

BaseViewWindow::BaseViewWindow(
	const char* title, int width, int height, GLFWmonitor* monitor, GLFWwindow* share
) : m_width(width), m_height(height) {

	/************** SET UP KEYBINDS **************/

	auto map_movement = [this](int action) {
		//mval determines whether motion in a direction should start or stop
		int mval = 0;
		switch (action)
		{
			case GLFW_PRESS:   mval =  1; break;
			case GLFW_RELEASE: mval = -1; break;
		}

		vec3& dir = this->m_cameraManager.cam_motion_dir;

		this->m_keyManager.mapKey(GLFW_KEY_W, action, 
			[&dir, mval]() {*dir[2] += +mval; });
		this->m_keyManager.mapKey(GLFW_KEY_A, action, 
			[&dir, mval]() {*dir[0] += -mval; });
		this->m_keyManager.mapKey(GLFW_KEY_S, action, 
			[&dir, mval]() {*dir[2] += -mval; });
		this->m_keyManager.mapKey(GLFW_KEY_D, action, 
			[&dir, mval]() {*dir[0] += +mval; });
		this->m_keyManager.mapKey(GLFW_KEY_LEFT_SHIFT, action, 
			[&dir, mval]() {*dir[1] += -mval; });
		this->m_keyManager.mapKey(GLFW_KEY_SPACE, action, 
			[&dir, mval]() {*dir[1] += mval; });
	};

	map_movement(GLFW_PRESS);
	map_movement(GLFW_RELEASE);

	// Press ESC to enable or disable camera controls
	m_keyManager.mapKey(GLFW_KEY_ESCAPE, GLFW_PRESS, [this]()
	{ 
		if (m_state.control_state == WinState::CONTROL_CAMERA) 
		{
			disableCameraControls(); 
			m_state.control_state = WinState::CONTROL_GUI;
		}
		else  
		{
			enableCameraControls(); 
			m_state.control_state = WinState::CONTROL_CAMERA;
		}
	});

	/************** SET UP CAMERA **************/
	m_camera = Camera(
		vec3({ -1,0,0 }),
		vec3({ 10,0,0 }),
		m_width,
		m_height,
		PI / 4);
	m_cameraManager.attach(&m_camera);


	// Create glfw context

	m_window = glfwCreateWindow(m_width, m_height, title, monitor, share);
	if (!m_window) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
	}
	glfwMakeContextCurrent(m_window);
	glfwSetWindowPos(m_window,0,0);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	glfwSetWindowUserPointer(m_window, this);
	glfwSetKeyCallback(m_window, keyCallback);
	glfwSetCursorPosCallback(m_window, cursorPosCallback);
	
	const GLubyte* _renderer = glGetString(GL_RENDERER);
	const GLubyte* _version = glGetString(GL_VERSION);

	printf("Renderer: %s\n", _renderer);
	printf("OpenGL version supported %s\n", _version);

	m_camera.init();
	m_state.is_running = true;
	m_cameraManager.start(&this->m_state);
}

BaseViewWindow::~BaseViewWindow()
{
	disableCameraControls();
	m_cameraManager.stop();
	m_state.is_running = false;
	glfwDestroyWindow(m_window);
}

void BaseViewWindow::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	auto win = static_cast<BaseViewWindow*>(glfwGetWindowUserPointer(window));

	win->m_keyManager.callKeyFunc(key, action);
}

void BaseViewWindow::cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	auto win = static_cast<BaseViewWindow*>(glfwGetWindowUserPointer(window));
	int width, height;
    glfwGetWindowSize(window, &width, &height);

	auto dx = xpos - width/2;
	auto dy = ypos - height/2;

	if (win->m_state.control_state == WinState::CONTROL_CAMERA) {
		win->m_cameraManager.rotate(dx,dy);
		glfwSetCursorPos(window,width/2,height/2);
	}	
}

void BaseViewWindow::enableCameraControls()
{
	//Center cursor so camera does not jerk
	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (glfwRawMouseMotionSupported()) {
		glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}
	glfwSetCursorPos(m_window,m_width/2,m_height/2);
	m_state.control_state = WinState::CONTROL_CAMERA;
	return;
}

void BaseViewWindow::disableCameraControls()
{
	m_state.control_state = WinState::CONTROL_GUI;
	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	if (glfwRawMouseMotionSupported()) {
		glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
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

void CameraManager::updateThread(WinState* state)
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
	updater = thread(&CameraManager::updateThread, this, state);
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

