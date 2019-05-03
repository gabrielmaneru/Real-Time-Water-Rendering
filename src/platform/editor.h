#pragma once
#include <string>
class c_editor
{
	void draw_main_window();

public:
	bool init();
	void update();
	void shutdown();

};

extern c_editor* editor;