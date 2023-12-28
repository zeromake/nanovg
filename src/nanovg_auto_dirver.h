
#ifndef NANOVG_AUTO_DIRVER_H
#define NANOVG_AUTO_DIRVER_H

#ifdef ANDROID
#define NANOVG_USE_GL 1
#include <android/log.h>
#if __ANDROID_API__ >= 24
#include <GLES3/gl32.h>
#elif __ANDROID_API__ >= 21
#include <GLES3/gl31.h>
#elif __ANDROID_API__ >= 18
#include <GLES3/gl3.h>
#else
#include <GLES2/gl2.h>
#define NANOVG_USE_GLES2 1
#endif
#ifndef NANOVG_USE_GLES2
#define NANOVG_USE_GLES3 1
#endif
#elif defined(_WIN32)
#define NANOVG_USE_D3D11 1
#elif defined(__APPLE__)
#define NANOVG_USE_METAL 1
#elif defined(__linux__)
#define NANOVG_USE_GL 1
#define NANOVG_USE_GL3 1
#ifdef NANOVG_GLEW
#include <GL/glew.h>
#endif
#endif

#endif
