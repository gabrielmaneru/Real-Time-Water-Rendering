#include "session.h"
int main()
{
	if (!session::init())
		return 1;
	session::update();
	session::shutdown();
	return 0;
}