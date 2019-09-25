/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	ortho_camera.h
Purpose: Basic orthogonal camera
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#pragma once
#include "camera.h"
class ortho_camera : public camera
{
public:
	ortho_camera(vec3 eye=def_cam.eye,
		vec3 front=def_cam.front,
		vec3 up=def_cam.up,
		vec4 view_rect=def_cam.viewrect);
	void update()override;

private:
	vec4 m_view_rect;
};