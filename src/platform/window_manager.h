/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	window_manager.h
Purpose: Window Manager
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

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