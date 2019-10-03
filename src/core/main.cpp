/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	main.cpp
Purpose: Program base
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#include "session.h"
#include <Windows.h>
#ifdef _DEBUG
int main()
#else
int CALLBACK WinMain(
	__in  HINSTANCE hInstance,
	__in  HINSTANCE hPrevInstance,
	__in  LPSTR lpCmdLine,
	__in  int nCmdShow
)
#endif
{
	if (!session::init())
		return 1;
	session::update();
	session::shutdown();
	return 0;
}