#pragma once
#include <string>
class c_editor
{
	void draw_main_window();

public:
	bool init();
	void update();
	void shutdown();

	int m_test_var{0};
};

extern c_editor* editor;