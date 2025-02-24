/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	session.cpp
Purpose: Session Controller
Author: Gabriel Ma�eru - gabriel.m
- End Header --------------------------------------------------------*/

#include "session.h"
#include <platform/window_manager.h>
#include <graphics/renderer.h>
#include <scene/scene.h>
#include <platform/editor.h>

bool session::end{false};
bool session::init()
{
	if (!window_manager->init()) return false;
	if (!editor->init()) return false;
	if (!renderer->init()) return false;
	if (!scene->init()) return false;
	scene->shutdown();
	if (!scene->init()) return false;
	return true;
}

void session::update()
{
	do
	{
		scene->update();
		renderer->update();
		editor->update();
		window_manager->update();
	} while (!end);
}

void session::shutdown()
{
	scene->shutdown();
	renderer->shutdown();
	editor->shutdown();
	window_manager->shutdown();
}
