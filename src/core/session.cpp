#include "session.h"
#include <platform/window_manager.h>
#include <graphics/renderer.h>
#include <scene/scene.h>

bool session::end{false};
bool session::init()
{
	if (!window_manager->init()) return false;
	if (!renderer->init()) return false;
	if (!scene->init()) return false;
	return true;
}

void session::update()
{
	do
	{
		scene->update();
		renderer->update();
		window_manager->update();
	} while (!end);
}

void session::shutdown()
{
	scene->shutdown();
	renderer->shutdown();
	window_manager->shutdown();
}
