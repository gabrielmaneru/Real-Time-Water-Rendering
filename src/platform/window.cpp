#include "window.h"
#include <core\session.h>
#include <GLFW\glfw3.h>

int window::m_keyboard[keyboard_size]{ 0 };
std::unordered_set<int> window::m_updated_keys{};
bool window::mouse_but_left_pressed{ false };
bool window::mouse_but_left_triggered{ false };
bool window::mouse_but_right_pressed{ false };
bool window::mouse_but_right_triggered{ false };
float window::mouse_offset[2]{ 0.0f };

static void glfw_key_callback(GLFWwindow*, int key, int, int action, int)
{
	if (key > 0)
	{
		if (action == GLFW_PRESS)
			window::m_keyboard[key-1] = 1;
		else if (action == GLFW_RELEASE)
			window::m_keyboard[key-1] = 0;
		window::m_updated_keys.insert(key);
	}
}

static bool firstMouse{ true };
static double lastX{ 0.0 };
static double lastY{ 0.0 };
static void glfw_mouse_callback(GLFWwindow*, double xpos, double ypos)
{
	if (firstMouse)
		lastX = xpos, lastY = ypos, firstMouse = false;

	window::mouse_offset[0] = static_cast<float>(xpos - lastX);
	window::mouse_offset[1] = static_cast<float>(lastY - ypos);
	lastX = xpos, lastY = ypos;
}

static void glfw_mouse_button_callback(GLFWwindow*, int button, int action, int)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		window::mouse_but_left_triggered = window::mouse_but_left_pressed = true;
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		window::mouse_but_right_triggered = window::mouse_but_right_pressed = true;

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		window::mouse_but_left_pressed = false;
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
		window::mouse_but_right_pressed = false;
}

window * window::create_window(int width, int height, const char * title, bool fullscreen)
{
	GLFWwindow * m_win_handler = glfwCreateWindow(width, height, title, fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);
	if (!m_win_handler)
		return nullptr;
	glfwMakeContextCurrent(m_win_handler);
	glfwSetKeyCallback(m_win_handler, glfw_key_callback);
	glfwSetCursorPosCallback(m_win_handler, glfw_mouse_callback);
	glfwSetMouseButtonCallback(m_win_handler, glfw_mouse_button_callback);
	return new window{ m_win_handler, width, height, title};
}

void window::update_window()
{
	glfwSwapBuffers(m_window);
	m_updated_keys.clear();
	mouse_but_left_triggered = false;
	mouse_but_right_triggered = false;
	window::mouse_offset[0] = window::mouse_offset[1] = 0.0f;
	
	glfwPollEvents();
	if (glfwWindowShouldClose(m_window))
		session::end |= true;

	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	if (h > 0)
		m_width = w, m_height = h;
}
