/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	gl_error.cpp
Purpose: OpenGL error handler
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#include "gl_error.h"
#include <iostream>

static void APIENTRY openglCallbackFunction(GLenum /*source*/, GLenum type, GLuint id,
	GLenum severity, GLsizei /*length*/, const GLchar* message, void* /*userParam*/)
{

	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;

	std::cout << "{\n";
	std::cout << "\tmessage: " << message << "\n";
	std::cout << "\ttype: ";
	switch (type) {
	case GL_DEBUG_TYPE_ERROR: std::cout << "ERROR";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "DEPRECATED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: std::cout << "UNDEFINED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_PORTABILITY: std::cout << "PORTABILITY";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE: std::cout << "PERFORMANCE";
		break;
	case GL_DEBUG_TYPE_OTHER: std::cout << "OTHER";
		break;
	default:break;
	}
	std::cout << "\n";

	std::cout << "\tid: " << id << "\n";
	std::cout << "\tseverity: ";
	switch (severity) {
	case GL_DEBUG_SEVERITY_LOW: std::cout << "LOW";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM: std::cout << "MEDIUM";
		break;
	case GL_DEBUG_SEVERITY_HIGH: std::cout << "HIGH";
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "NOTIFICATION";
		break;
	default:break;
	}
	std::cout << "\n}\n";
	assert(type != GL_DEBUG_TYPE_ERROR);
}

void setup_gl_debug()
{
	// Debug
	GL_CALL(glEnable(GL_DEBUG_OUTPUT));
	GL_CALL(glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS));
	GL_CALL(glDebugMessageCallback(&openglCallbackFunction, nullptr));
	GLuint unusedIds = 0;
	GL_CALL(glDebugMessageControl(GL_DONT_CARE,
		GL_DONT_CARE,
		GL_DONT_CARE,
		0,
		&unusedIds,
		GL_TRUE));
}
