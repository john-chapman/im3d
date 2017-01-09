#pragma once
#ifndef im3d_common_h
#define im3d_common_h

// Compiler
#if defined(__GNUC__)
	#define IM3D_COMPILER_GNU
#elif defined(_MSC_VER)
	#define IM3D_COMPILER_MSVC
#else
	#error im3d: Compiler not defined
#endif

// Platform 
#if defined(_WIN32) || defined(_WIN64)
	#define IM3D_PLATFORM_WIN
#else
	#error im3d: Platform not defined
#endif

// Graphics
#if defined(IM3D_GL)
#else
	#error im3d: Graphics lib not defined
#endif


#define IM3D_UNUSED(x) do { (void)sizeof(x); } while(0)
#ifdef IM3D_COMPILER_MSVC
	#define IM3D_BREAK() __debugbreak()
#else
	#include <cstdlib>
	#define IM3D_BREAK() abort()
#endif

#define IM3D_ASSERT_MSG(e, msg, ...) \
	do { \
		if (!(e)) { \
			Im3d::Assert(#e, __FILE__, __LINE__, msg, __VA_ARGS__); \
			IM3D_BREAK(); \
		} \
	} while (0)

#define IM3D_ASSERT(e)                 IM3D_ASSERT_MSG(e, 0, 0)
#define IM3D_VERIFY_MSG(e, msg, ...)   IM3D_ASSERT_MSG(e, msg, __VA_ARGS__)
#define IM3D_VERIFY(e)                 IM3D_VERIFY_MSG(e, 0, 0)


#if defined(IM3D_GL)
	#include "GL/glew.h"
	#define glAssert(call) \
	do { \
		(call); \
		GLenum err = glGetError(); \
		if (err != GL_NO_ERROR) { \
			Im3d::Assert(#call, __FILE__, __LINE__, GetGlEnumString(err)); \
			IM3D_BREAK(); \
		} \
	} while (0)
#endif

namespace Im3d {

void Assert(const char* _e, const char* _file, int _line, const char* _msg, ...);

const char* GetPlatformErrorString(void* _err);

#if defined(IM3D_GL)
	const char* GetGlEnumString(GLenum _enum);
	const char* GlGetString(GLenum _name);
#endif

} // namespac Im3d

#include "im3d.h"
#include "im3d_math.h"

#if defined(IM3D_PLATFORM_WIN)
	#define NOMINMAX 1
	#define WIN32_LEAN_AND_MEAN 1
	#define VC_EXTRALEAN 1
	#include <Windows.h>

	#define IM3D_PLATFORM_ASSERT(e) IM3D_ASSERT_MSG(e, GetPlatformErrorString((void*)GetLastError()))
	#define IM3D_PLATFORM_VERIFY(e) IM3D_VERIFY_MSG(e, GetPlatformErrorString((void*)GetLastError()))
#endif

#include "imgui/imgui.h"


void        RandSeed(int _seed);
float       RandFloat(float _min, float _max);
Im3d::Vec3  RandVec3(float _min, float _max);

#endif // im3d_common_h
