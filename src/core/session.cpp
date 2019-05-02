#include "session.h"
#include <platform\window_manager.h>
#include <graphics/renderer.h>

bool session::end{false};
void session::init()
{
	window_manager->init();
	renderer->init();
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
