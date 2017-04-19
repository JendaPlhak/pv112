#pragma once
#include <iostream>

#define _USE_MATH_DEFINES
#include <math.h>

// Include GLEW, use static library
#define GLEW_STATIC
#include <GL/glew.h>
#pragma comment(lib, "glew32s.lib") // Link with GLEW library

// Include DevIL for image loading
#if defined(_WIN32)
#pragma comment(lib, "glew32s.lib")
// On Windows, we use Unicode dll of DevIL
// That also means we need to use wide strings
#ifndef _UNICODE
#define _UNICODE
#include <IL/il.h>
#undef _UNICODE
#else
#include <IL/il.h>
#endif
#pragma comment(lib, "DevIL.lib") // Link with DevIL library
typedef wchar_t maybewchar;
#define MAYBEWIDE(s) L##s
#else // On Linux, we need regular (not wide) strings
#include <IL/il.h>
typedef char maybewchar;
#define MAYBEWIDE(s) s
#endif

// Include FreeGLUT
#include <GL/freeglut.h>

// Include the most important GLM functions
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
