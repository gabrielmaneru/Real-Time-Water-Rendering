/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	session.h
Purpose: Session Controller
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#pragma once

namespace session
{
	extern bool end;
	bool init();
	void update();
	void shutdown();
}