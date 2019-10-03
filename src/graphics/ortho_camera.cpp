/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	ortho_camera.cpp
Purpose: Basic orthogonal camera
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#include "ortho_camera.h"

ortho_camera::ortho_camera(vec3 eye, vec3 front, vec3 up)
	:camera( eye,front,up )
{
	m_view = mat4(1.0f);
	m_proj = mat4(1.0f);
}