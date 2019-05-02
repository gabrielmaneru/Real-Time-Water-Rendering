#pragma once

class c_renderer
{

public:
	bool init();
	void update();
	void shutdown();
};
extern c_renderer* renderer;