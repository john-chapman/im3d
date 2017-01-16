#pragma once
#ifndef im3d_example_h
#define im3d_example_h

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
 // Windows
	#define IM3D_PLATFORM_WIN
	
	#define NOMINMAX 1
	#define WIN32_LEAN_AND_MEAN 1
	#define VC_EXTRALEAN 1
	#include <Windows.h>

	#define winAssert(e) IM3D_VERIFY_MSG(e, Im3d::GetPlatformErrorString(GetLastError()))
	
	namespace Im3d {
		const char* GetPlatformErrorString(DWORD _err);
	}
	
#else
	#error im3d: Platform not defined
#endif

// Graphics API
#if defined(IM3D_OPENGL)
 // OpenGL
	#define IM3D_OPENGL_VMAJ    3
	#define IM3D_OPENGL_VMIN    3
	#define IM3D_OPENGL_VSHADER "#version 330"
 
	#include "GL/glew.h"
	#define glAssert(call) \
		do { \
			(call); \
			GLenum err = glGetError(); \
			if (err != GL_NO_ERROR) { \
				Im3d::Assert(#call, __FILE__, __LINE__, Im3d::GetGlEnumString(err)); \
				IM3D_BREAK(); \
			} \
		} while (0)
		
	namespace Im3d {
		// Return 0 on failure (prints log info to stderr). _defines is a list of null-separated strings e.g. "DEFINE1 1\0DEFINE2 1\0"
		GLuint LoadCompileShader(GLenum _stage, const char* _path, const char* _defines = 0);
		// Return false on failure (prints log info to stderr).
		bool LinkShaderProgram(GLuint _handle);
		
		const char* GetGlEnumString(GLenum _enum);
		const char* GlGetString(GLenum _name);
	}
	
#else
	#error im3d: Graphics API not defined
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

#ifndef __COUNTER__
	#define __COUNTER__ __LINE__
#endif
#define IM3D_TOKEN_CONCATENATE_(_t0, _t1) _t0 ## _t1
#define IM3D_TOKEN_CONCATENATE(_t0, _t1)  IM3D_TOKEN_CONCATENATE_(_t0, _t1)
#define IM3D_UNIQUE_NAME(_base) IM3D_TOKEN_CONCATENATE(_base, __COUNTER__)

#include "im3d.h"
#include "im3d_math.h"

#include "imgui/imgui.h"

namespace Im3d {

void        Assert(const char* _e, const char* _file, int _line, const char* _msg, ...);

void        RandSeed(int _seed);
int         RandInt(int _min, int _max);
float       RandFloat(float _min, float _max);
Im3d::Vec3  RandVec3(float _min, float _max);

struct Example
{
	
	bool init(int _width, int _height, const char* _title);
	void shutdown();
	bool update();
	void draw();
 
 // window 
	int m_width, m_height;
	const char* m_title;
	
	bool hasFocus() const;
	Vec2 getWindowRelativeCursor() const;
	
 // 3d camera
    Vec3  m_camPos;
	Vec3  m_camDir;
	float m_camFovDeg;
	float m_camFovRad;
	Mat4  m_camWorld;
	Mat4  m_camView;
	Mat4  m_camProj;
	Mat4  m_camViewProj;
	
	float m_deltaTime;

 // platform/graphics specifics
	#if defined(IM3D_PLATFORM_WIN)
		HWND m_hwnd;
		LARGE_INTEGER m_currTime, m_prevTime;
		
		#if defined(IM3D_OPENGL)
			HDC   m_hdc;
			HGLRC m_hglrc;
			
		#endif
	#endif

}; // struct Example

extern Example* g_Example;

} // namespace Im3d


// per-example implementations (in the example .cpp)
extern bool Im3d_Init();
extern void Im3d_Shutdown();
extern void Im3d_Update();
extern void Im3d_Draw(Im3d::DrawPrimitiveType _primType, const Im3d::VertexData* _data, Im3d::U32 _count);

#endif // im3d_example_h
