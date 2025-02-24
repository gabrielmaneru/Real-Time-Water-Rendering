/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	window_manager.h
Purpose: Window Manager
Author: Gabriel Ma�eru - gabriel.m
- End Header --------------------------------------------------------*/

#include "window_manager.h"
#include "window.h"
#include <core\session.h>
#include <GLFW\glfw3.h>

c_window_manager * window_manager = new c_window_manager;

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

bool c_window_manager::init()
{
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return false;

	m_window = window::create_window(1920, 1080, "Framework", false);

	if (!m_window) shutdown();
	return m_window != nullptr;
}

void c_window_manager::update()
{
	m_window->update_window();
	if (is_key_down(GLFW_KEY_LEFT_CONTROL) && is_key_triggered(GLFW_KEY_Q))
		session::end |= true;
}

void c_window_manager::shutdown()
{
	delete m_window;
	glfwTerminate();
}

int c_window_manager::get_width()
{
	return m_window->m_width;
}

int c_window_manager::get_height()
{
	return m_window->m_height;
}

bool c_window_manager::is_key_up(int key)
{
	return m_window->m_keyboard[key-1] == 0;
}

bool c_window_manager::is_key_down(int key)
{
	return m_window->m_keyboard[key-1] == 1;
}

bool c_window_manager::is_key_triggered(int key)
{
	if (is_key_down(key))
	{
		if (m_window->m_updated_keys.find(key) != m_window->m_updated_keys.end())
			return true;
	}
	return false;
}

bool c_window_manager::is_key_released(int key)
{
	if (is_key_up(key))
	{
		if (m_window->m_updated_keys.find(key) != m_window->m_updated_keys.end())
			return true;
	}
	return false;
}
