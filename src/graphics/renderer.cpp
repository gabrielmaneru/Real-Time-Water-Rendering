#include "renderer.h"
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

c_renderer* renderer = new c_renderer;
bool c_renderer::init()
{
	if (gl3wInit())
		return false;
	
	if (!gl3wIsSupported(4, 0))
		return false;

	// GL Options
	GL_CALL(glClearColor(0.1f, 0.1f, 0.1f, 1.0f));

	//Load Programs

	//Load Resources

	return true;
}

void c_renderer::update()
{
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	// Camera Update

	// Scene Draw

}

void c_renderer::shutdown()
{
	//Clean Resouces
}
