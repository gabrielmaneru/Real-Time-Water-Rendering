/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	gl_error.h
Purpose: OpenGL error handler
Author: Gabriel Ma�eru - gabriel.m
- End Header --------------------------------------------------------*/

#pragma once
#include <GL\gl3w.h>
#include <assert.h>
#define GL_CALL(what) {what; checkGlError();}
inline void checkGlError()
{
	const auto iErr = glGetError();
	if (iErr != GL_NO_ERROR) {
		const char* pszError;
		switch (iErr) {
		case GL_INVALID_ENUM: {
			pszError = "GL_INVALID_ENUM";
			break;
		}
		case GL_INVALID_VALUE: {
			pszError = "GL_INVALID_VALUE";
			break;
		};
		case GL_INVALID_OPERATION: {
			pszError = "GL_INVALID_OPERATION";
			break;
		};
		case GL_INVALID_FRAMEBUFFER_OPERATION: {
			pszError = "GL_INVALID_FRAMEBUFFER_OPERATION";
			break;
		};
		case GL_OUT_OF_MEMORY: {
			pszError = "GL_OUT_OF_MEMORY";
			break;
		};
#if defined(GL_STACK_UNDERFLOW) && defined(GL_STACK_OVERFLOW)
		case GL_STACK_UNDERFLOW: {
			pszError = "GL_STACK_UNDERFLOW";
			break;
		};
		case GL_STACK_OVERFLOW: {
			pszError = "GL_STACK_OVERFLOW";
			break;
		};
#endif
		default: {pszError = "Unknown"; }
		}
		assert(false);
	}
}
/**
 *
 */
void setup_gl_debug();