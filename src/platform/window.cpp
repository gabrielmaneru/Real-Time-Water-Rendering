#include "window.h"
#include <core\session.h>
#include <GLFW\glfw3.h>

int window::m_keyboard[keyboard_size]{ 0 };
std::unordered_set<int> window::m_updated_keys{};

static void glfw_key_callback(GLFWwindow*, int key, int, int action, int)
{
	if (key > 0)
	{
		if (action == GLFW_PRESS)
			window::m_keyboard[key - 1] = 1;
		else if (action == GLFW_RELEASE)
			window::m_keyboard[key - 1] = 0;
		window::m_updated_keys.insert(key);
	}
}

window * window::create_window(int width, int height, const char * title, bool fullscreen)
{
	GLFWwindow * m_win_handler = glfwCreateWindow(width, height, title, fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);
	if (!m_win_handler)
		return nullptr;
	glfwMakeContextCurrent(m_win_handler);
	glfwSetKeyCallback(m_win_handler, glfw_key_callback);
	return new window{ m_win_handler, width, height, title};
}

void window::update_window()
{
	glfwSwapBuffers(m_window);
	m_updated_keys.clear();
	glfwPollEvents();
	if (glfwWindowShouldClose(m_window))
		session::end |= true;
}
