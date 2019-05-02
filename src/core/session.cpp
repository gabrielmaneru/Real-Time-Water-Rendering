#include "session.h"
#include <platform\window_manager.h>
#include <graphics/renderer.h>

bool session::end{false};
bool session::init()
{
	if (!window_manager->init()) return false;
	if (!renderer->init()) return false;
	return true;
}

void session::update()
{
	do
	{
		window_manager->update();
		renderer->update();
	} while (!end);
}

void session::shutdown()
{
	renderer->shutdown();
	window_manager->shutdown();
}
