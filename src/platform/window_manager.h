#pragma once

struct window;

class c_window_manager
{
public:
	bool init();
	void update();
	void shutdown();

	window * m_window{ nullptr };

	int get_width();
	int get_height();

	bool is_key_up(int key);
	bool is_key_down(int key);
	bool is_key_triggered(int key);
	bool is_key_released(int key);
};
extern c_window_manager * window_manager;