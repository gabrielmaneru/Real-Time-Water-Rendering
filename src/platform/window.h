#pragma once
#include <unordered_set>
struct GLFWwindow;
constexpr int keyboard_size = 348;
struct window
{
	static window * create_window(int width, int height, const char * title, bool fullscreen);
	void update_window();

	GLFWwindow * m_window;
	int m_width;
	int m_height;
	const char * m_title;

	static std::unordered_set<int> m_updated_keys;
	static int m_keyboard[keyboard_size];
	static bool mouse_but_left_pressed;
	static bool mouse_but_left_triggered;
	static bool mouse_but_right_pressed;
	static bool mouse_but_right_triggered;
	static float mouse_offset[2];
	friend class c_editor;
};