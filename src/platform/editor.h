/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	editor.h
Purpose: Window
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#pragma once
#include <string>
class c_editor
{
	void draw_main_window();

public:
	bool init();
	void update();
	void shutdown();

	float m_test_var;
};

extern c_editor* editor;