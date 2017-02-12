#include "im3d_example.h"

#include "teapot.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace Im3d;

#ifdef IM3D_COMPILER_MSVC
	#pragma warning(disable: 4996) // vsnprintf

	#pragma warning(disable: 4311) // typecast
	#pragma warning(disable: 4302) //    "
	#pragma warning(disable: 4312) //    "
#endif

/******************************************************************************/
#if defined(IM3D_PLATFORM_WIN)
	static LARGE_INTEGER g_SysTimerFreq;

	const char* Im3d::GetPlatformErrorString(DWORD _err)
	{
		const int kErrMsgMax = 1024;
		static char buf[kErrMsgMax];
		buf[0] = '\0';
		IM3D_VERIFY(
			FormatMessage(
				FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
				NULL, 
				_err,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)buf, 
				kErrMsgMax, 
				NULL
			) != 0
		);
		return buf;
	}
	
	static LRESULT CALLBACK WindowProc(HWND _hwnd, UINT _umsg, WPARAM _wparam, LPARAM _lparam)
	{
		ImGuiIO& imgui = ImGui::GetIO();
		Example* im3d = g_Example;
		
		switch (_umsg) {
		case WM_SIZE: {
			int w = (int)LOWORD(_lparam), h = (int)HIWORD(_lparam);
			if (im3d->m_width != w || im3d->m_height != h) {
				im3d->m_width = w;
				im3d->m_height = h;
			}
			break;
		}
		case WM_SIZING: {
			RECT* r = (RECT*)_lparam;
			int w = (int)(r->right - r->left);
			int h = (int)(r->bottom - r->top);
			if (im3d->m_width != w || im3d->m_height != h) {
				im3d->m_width = w;
				im3d->m_height = h;
			}
			break;
		}
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
			imgui.MouseDown[0] = _umsg == WM_LBUTTONDOWN;
			break;
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
			imgui.MouseDown[2] = _umsg == WM_MBUTTONDOWN;
			break;
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			imgui.MouseDown[1] = _umsg == WM_RBUTTONDOWN;
			break;
		case WM_MOUSEWHEEL:
			imgui.MouseWheel = (float)(GET_WHEEL_DELTA_WPARAM(_wparam)) / (float)(WHEEL_DELTA); 
			break;
		case WM_MOUSEMOVE:
			imgui.MousePos.x = LOWORD(_lparam);
			imgui.MousePos.y = HIWORD(_lparam);
			break;
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP: {
			WPARAM vk = _wparam;
			UINT sc = (_lparam & 0x00ff0000) >> 16;
			bool e0 = (_lparam & 0x01000000) != 0;
			if (vk == VK_SHIFT) {
				vk = MapVirtualKey(sc, MAPVK_VSC_TO_VK_EX);
			}
			switch (vk) {
			case VK_CONTROL:
				imgui.KeyCtrl = _umsg == WM_KEYDOWN;
				break;
			case VK_MENU:
				imgui.KeyAlt = _umsg == WM_KEYDOWN;
				break;
			case VK_LSHIFT:
			case VK_RSHIFT:
				imgui.KeyShift = _umsg == WM_KEYDOWN;
				break;
			case VK_ESCAPE:
				PostQuitMessage(0);
				break;
			default:
				if (vk < 512) {
					imgui.KeysDown[vk] = _umsg == WM_KEYDOWN;
				}
				break;
			};
			return 0;
		}
		case WM_CHAR:
			if (_wparam > 0 && _wparam < 0x10000) {
				imgui.AddInputCharacter((unsigned short)_wparam);
			}
			return 0;
		case WM_PAINT:
			//IM3D_ASSERT(false); // should be suppressed by calling ValidateRect()
			break;
		case WM_CLOSE:
			PostQuitMessage(0);
			return 0; // prevent DefWindowProc from destroying the window
		default:
			break;
		};
		return DefWindowProc(_hwnd, _umsg, _wparam, _lparam);
	}
	
	static bool InitWindow(int& _width_, int& _height_, const char* _title)
	{
		static ATOM wndclassex = 0;
		if (wndclassex == 0) {
			WNDCLASSEX wc;
			memset(&wc, 0, sizeof(wc));
			wc.cbSize = sizeof(wc);
			wc.style = CS_OWNDC;// | CS_HREDRAW | CS_VREDRAW;
			wc.lpfnWndProc = WindowProc;
			wc.hInstance = GetModuleHandle(0);
			wc.lpszClassName = "Im3dTestApp";
			wc.hCursor = LoadCursor(0, IDC_ARROW);
			winAssert(wndclassex = RegisterClassEx(&wc));
		}
	
		DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_MINIMIZEBOX | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	
		if (_width_ == -1 || _height_ == -1) {
		// auto size; get the dimensions of the primary screen area and subtract the non-client area
			RECT r;
			winAssert(SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0));
			_width_  = r.right - r.left;
			_height_ = r.bottom - r.top;
	
			RECT wr = {};
			winAssert(AdjustWindowRectEx(&wr, dwStyle, FALSE, dwExStyle));
			_width_  -= wr.right - wr.left;
			_height_ -= wr.bottom - wr.top;
		}
	
		RECT r; r.top = 0; r.left = 0; r.bottom = _height_; r.right = _width_;
		winAssert(AdjustWindowRectEx(&r, dwStyle, FALSE, dwExStyle));
		g_Example->m_hwnd = CreateWindowEx(
			dwExStyle, 
			MAKEINTATOM(wndclassex), 
			_title, 
			dwStyle, 
			0, 0, 
			r.right - r.left, r.bottom - r.top, 
			NULL, 
			NULL, 
			GetModuleHandle(0), 
			NULL
			);
		IM3D_ASSERT(g_Example->m_hwnd);
		ShowWindow(g_Example->m_hwnd, SW_SHOW);
		return true;
	}
	
	static void ShutdownWindow()
	{
		if (g_Example->m_hwnd) {
			winAssert(DestroyWindow(g_Example->m_hwnd));
		}
	}
	
	#if defined(IM3D_OPENGL)
		#include "GL/wglew.h"
		static PFNWGLCHOOSEPIXELFORMATARBPROC    wglChoosePixelFormat    = 0;
		static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribs = 0;

		
		static bool InitOpenGL(int _vmaj, int _vmin)
		{
		 // create dummy context for extension loading
			static ATOM wndclassex = 0;
			if (wndclassex == 0) {
				WNDCLASSEX wc;
				memset(&wc, 0, sizeof(wc));
				wc.cbSize = sizeof(wc);
				wc.style = CS_OWNDC;// | CS_HREDRAW | CS_VREDRAW;
				wc.lpfnWndProc = DefWindowProc;
				wc.hInstance = GetModuleHandle(0);
				wc.lpszClassName = "Im3dTestApp_GlDummy";
				wc.hCursor = LoadCursor(0, IDC_ARROW);
				winAssert(wndclassex = RegisterClassEx(&wc));
			}
			HWND hwndDummy = CreateWindowEx(0, MAKEINTATOM(wndclassex), 0, NULL, 0, 0, 1, 1, NULL, NULL, GetModuleHandle(0), NULL);
			winAssert(hwndDummy);
			HDC hdcDummy = 0;
			winAssert(hdcDummy = GetDC(hwndDummy));	
			PIXELFORMATDESCRIPTOR pfd;
			memset(&pfd, 0, sizeof(pfd));
			winAssert(SetPixelFormat(hdcDummy, 1, &pfd));
			HGLRC hglrcDummy = 0;
			winAssert(hglrcDummy = wglCreateContext(hdcDummy));
			winAssert(wglMakeCurrent(hdcDummy, hglrcDummy));
			
		// check the platform supports the requested GL version
			GLint platformVMaj, platformVMin;
			glAssert(glGetIntegerv(GL_MAJOR_VERSION, &platformVMaj));
			glAssert(glGetIntegerv(GL_MINOR_VERSION, &platformVMin));
			if (platformVMaj < _vmaj || (platformVMaj >= _vmaj && platformVMin < _vmin)) {
				fprintf(stderr, "OpenGL version %d.%d is not available (max version is %d.%d)\n", _vmaj, _vmin, platformVMaj, platformVMin);
				fprintf(stderr, "\t(This error may occur if the platform has an integrated GPU)\n");
				return false;
			}
		
		// load wgl extensions for true context creation
			winAssert(wglChoosePixelFormat = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB"));
			winAssert(wglCreateContextAttribs = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB"));
		
		// delete the dummy context
			winAssert(wglMakeCurrent(0, 0));
			winAssert(wglDeleteContext(hglrcDummy));
			winAssert(ReleaseDC(hwndDummy, hdcDummy) != 0);
			winAssert(DestroyWindow(hwndDummy) != 0);
		
		// create true context
			winAssert(g_Example->m_hdc = GetDC(g_Example->m_hwnd));
			const int pfattr[] = {
				WGL_SUPPORT_OPENGL_ARB, 1,
				WGL_DRAW_TO_WINDOW_ARB, 1,
				WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
				WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
				WGL_DOUBLE_BUFFER_ARB,  1,
				WGL_COLOR_BITS_ARB,     24,
				WGL_ALPHA_BITS_ARB,     8,
				WGL_DEPTH_BITS_ARB,     16,
				WGL_STENCIL_BITS_ARB,   0,
				0
			};
			int pformat = -1, npformat = -1;
			winAssert(wglChoosePixelFormat(g_Example->m_hdc, pfattr, 0, 1, &pformat, (::UINT*)&npformat));
			winAssert(SetPixelFormat(g_Example->m_hdc, pformat, &pfd));
			int profileBit = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
			//profileBit = WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
			int attr[] = {
				WGL_CONTEXT_MAJOR_VERSION_ARB,	_vmaj,
				WGL_CONTEXT_MINOR_VERSION_ARB,	_vmin,
				WGL_CONTEXT_PROFILE_MASK_ARB,	profileBit,
				0
			};
			winAssert(g_Example->m_hglrc = wglCreateContextAttribs(g_Example->m_hdc, 0, attr));
		
		// load extensions
			if (!wglMakeCurrent(g_Example->m_hdc, g_Example->m_hglrc)) {
				fprintf(stderr, "wglMakeCurrent failed");
				return false;
			}
			glewExperimental = GL_TRUE;
			GLenum err = glewInit();
			IM3D_ASSERT(err == GLEW_OK);
			glGetError(); // clear any errors caused by glewInit()

			winAssert(wglSwapIntervalEXT(0)); // example uses FPS as a rough perf measure, hence disable vsync

			fprintf(stdout, "OpenGL context:\n\tVersion: %s\n\tGLSL Version: %s\n\tVendor: %s\n\tRenderer: %s\n",
				GlGetString(GL_VERSION),
				GlGetString(GL_SHADING_LANGUAGE_VERSION),
				GlGetString(GL_VENDOR),
				GlGetString(GL_RENDERER)
				);
			return true;
		}
		
		static void ShutdownOpenGL()
		{
			winAssert(wglMakeCurrent(0, 0));
			winAssert(wglDeleteContext(g_Example->m_hglrc));
			winAssert(ReleaseDC(g_Example->m_hwnd, g_Example->m_hdc) != 0);
		}
		
	#endif // graphics
	
#endif // platform

/******************************************************************************/
#if defined(IM3D_OPENGL)
	static void Append(const char* _str, Vector<char>& _out_)
	{
		while (*_str) {
			_out_.push_back(*_str);
			++_str;
		}
	}
	static void AppendLine(const char* _str, Vector<char>& _out_)
	{
		Append(_str, _out_);
		_out_.push_back('\n');
	}
	
	GLuint Im3d::LoadCompileShader(GLenum _stage, const char* _path, const char* _defines)
	{
		Vector<char> src;
		AppendLine(IM3D_OPENGL_VSHADER, src);
		if (_defines) {
			while (*_defines != '\0') {
				Append("#define ", src);
				AppendLine(_defines, src);
				_defines = strchr(_defines, 0);
				IM3D_ASSERT(_defines);
				++_defines;
			}
		}
	
		FILE* fin = fopen(_path, "rb");
		if (!fin) {
			fprintf(stderr, "Error opening '%s'\n", _path);
			return 0;
		}
		IM3D_VERIFY(fseek(fin, 0, SEEK_END) == 0); // not portable but should work almost everywhere
		long fsize = ftell(fin);
		IM3D_VERIFY(fseek(fin, 0, SEEK_SET) == 0);
	
		int srcbeg = src.size();
		src.resize(srcbeg + fsize, '\0');
		if (fread(src.data() + srcbeg, 1, fsize, fin) != fsize) {
			fclose(fin);
			fprintf(stderr, "Error reading '%s'\n", _path);
			return 0;
		}
		fclose(fin);
		src.push_back('\0');
	
		GLuint ret = 0;
		glAssert(ret = glCreateShader(_stage));
		const GLchar* pd = src.data();
		GLint ps = src.size();
		glAssert(glShaderSource(ret, 1, &pd, &ps));
	
		glAssert(glCompileShader(ret));
		GLint compileStatus = GL_FALSE;
		glAssert(glGetShaderiv(ret, GL_COMPILE_STATUS, &compileStatus));
		if (compileStatus == GL_FALSE) {
			fprintf(stderr, "Error compiling '%s':\n\n", _path);
			GLint len;
			glAssert(glGetShaderiv(ret, GL_INFO_LOG_LENGTH, &len));
			char* log = new GLchar[len];
			glAssert(glGetShaderInfoLog(ret, len, 0, log));
			fprintf(stderr, log);
			delete[] log;
	
			//fprintf(stderr, "\n\n%s", src.data());
			fprintf(stderr, "\n");
			glAssert(glDeleteShader(ret));
			return 0;
		}
		return ret;
	}
	
	bool Im3d::LinkShaderProgram(GLuint _handle)
	{
		IM3D_ASSERT(_handle != 0);
	
		glAssert(glLinkProgram(_handle));
		GLint linkStatus = GL_FALSE;
		glAssert(glGetProgramiv(_handle, GL_LINK_STATUS, &linkStatus));
		if (linkStatus == GL_FALSE) {
			fprintf(stderr, "Error linking program:\n\n");
			GLint len;
			glAssert(glGetProgramiv(_handle, GL_INFO_LOG_LENGTH, &len));
			GLchar* log = new GLchar[len];
			glAssert(glGetProgramInfoLog(_handle, len, 0, log));
			fprintf(stderr, log);
			fprintf(stderr, "\n");
			delete[] log;
	
			return false;
		}
		return true;
	}

	void Im3d::DrawNdcQuad()
	{
		static GLuint vbQuad;
		static GLuint vaQuad;
		if (vbQuad == 0) {
			float quadv[8] = {
				-1.0f, -1.0f,
				 1.0f, -1.0f,
				-1.0f,  1.0f,
				 1.0f,  1.0f,
				};
			glAssert(glCreateBuffers(1, &vbQuad));
			glAssert(glCreateVertexArrays(1, &vaQuad));	
			glAssert(glBindVertexArray(vaQuad));
			glAssert(glBindBuffer(GL_ARRAY_BUFFER, vbQuad));
			glAssert(glEnableVertexAttribArray(0));
			glAssert(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (GLvoid*)0));
			glAssert(glBufferData(GL_ARRAY_BUFFER, sizeof(quadv), (GLvoid*)quadv, GL_STATIC_DRAW));
			glAssert(glBindVertexArray(0));	
		}
		glAssert(glBindVertexArray(vaQuad));
		glAssert(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
		glAssert(glBindVertexArray(0));
	}

	void Im3d::DrawTeapot(const Mat4& _world, const Mat4& _viewProj)
	{
		static GLuint shTeapot;
		static GLuint vbTeapot;
		static GLuint ibTeapot;
		static GLuint vaTeapot;
		if (shTeapot == 0) {
			GLuint vs = LoadCompileShader(GL_VERTEX_SHADER,   "model.glsl", "VERTEX_SHADER\0");
			GLuint fs = LoadCompileShader(GL_FRAGMENT_SHADER, "model.glsl", "FRAGMENT_SHADER\0");
			if (vs && fs) {
				glAssert(shTeapot = glCreateProgram());
				glAssert(glAttachShader(shTeapot, vs));
				glAssert(glAttachShader(shTeapot, fs));
				bool ret = LinkShaderProgram(shTeapot);
				glAssert(glDeleteShader(vs));
				glAssert(glDeleteShader(fs));
				if (!ret) {
					return;
				}
			} else {
				return;
			}
			glAssert(glCreateBuffers(1, &vbTeapot));
			glAssert(glCreateBuffers(1, &ibTeapot));
			glAssert(glCreateVertexArrays(1, &vaTeapot));	
			glAssert(glBindVertexArray(vaTeapot));
			glAssert(glBindBuffer(GL_ARRAY_BUFFER, vbTeapot));
			glAssert(glEnableVertexAttribArray(0));
			glAssert(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3) * 2, (GLvoid*)0));
			glAssert(glEnableVertexAttribArray(1));
			glAssert(glEnableVertexAttribArray(0));
			glAssert(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (GLvoid*)0));
			glAssert(glEnableVertexAttribArray(1));
			glAssert(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (GLvoid*)(sizeof(float) * 3)));
			glAssert(glBufferData(GL_ARRAY_BUFFER, sizeof(s_teapotVertices), (GLvoid*)s_teapotVertices, GL_STATIC_DRAW));
			glAssert(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibTeapot));
			glAssert(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(s_teapotIndices), (GLvoid*)s_teapotIndices, GL_STATIC_DRAW));
			glAssert(glBindVertexArray(0));
		}
		glAssert(glUseProgram(shTeapot));
		glAssert(glUniformMatrix4fv(glGetUniformLocation(shTeapot, "uWorldMatrix"), 1, false, _world.m));
		glAssert(glUniformMatrix4fv(glGetUniformLocation(shTeapot, "uViewProjMatrix"), 1, false, _viewProj.m));
		glAssert(glBindVertexArray(vaTeapot));
		glAssert(glEnable(GL_DEPTH_TEST));
		glAssert(glEnable(GL_CULL_FACE));
		glAssert(glDrawElements(GL_TRIANGLES, sizeof(s_teapotIndices) / sizeof(unsigned), GL_UNSIGNED_INT, (GLvoid*)0));
		glAssert(glDisable(GL_DEPTH_TEST));
		glAssert(glDisable(GL_CULL_FACE));
		glAssert(glBindVertexArray(0));
		glAssert(glUseProgram(0));
	}
	
	const char* Im3d::GetGlEnumString(GLenum _enum)
	{
		#define CASE_ENUM(e) case e: return #e
		switch (_enum) {
		// errors
			CASE_ENUM(GL_NONE);
			CASE_ENUM(GL_INVALID_ENUM);
			CASE_ENUM(GL_INVALID_VALUE);
			CASE_ENUM(GL_INVALID_OPERATION);
			CASE_ENUM(GL_INVALID_FRAMEBUFFER_OPERATION);
			CASE_ENUM(GL_OUT_OF_MEMORY);
	
			default: return "Unknown GLenum";
		};
		#undef CASE_ENUM
	}
	
	const char* Im3d::GlGetString(GLenum _name)
	{
		const char* ret;
		glAssert(ret = (const char*)glGetString(_name));
		return ret ? ret : "";
	}
	
#endif // graphics

/******************************************************************************/
static const char* StripPath(const char* _path) 
{
	int i = 0, last = 0;
	while (_path[i] != '\0') {
		if (_path[i] == '\\' || _path[i] == '/') {
			last = i + 1;
		}
		++i;
	}
	return &_path[last];
}

void Im3d::Assert(const char* _e, const char* _file, int _line, const char* _msg, ...)
{
	const int kAssertMsgMax = 1024;

	char buf[kAssertMsgMax];
	if (_msg != nullptr) {
		va_list args;
		va_start(args, _msg);
		vsnprintf(buf, kAssertMsgMax, _msg, args);
		va_end(args);
	} else {
		buf[0] = '\0';
	}
	fprintf(stderr, "Assert (%s, line %d)\n\t'%s' %s", StripPath(_file), _line, _e ? _e : "", buf);
}

void Im3d::RandSeed(int _seed)
{
	srand(_seed);
}
int Im3d::RandInt(int _min, int _max)
{
	return _min + (int)rand() % (_max - _min);
}
float Im3d::RandFloat(float _min, float _max)
{
	return _min + (float)rand() / (float)RAND_MAX * (_max - _min);
}
Mat3 Im3d::RandRotation()
{
	return Rotation(Normalize(RandVec3(-1.0f, 1.0f)), RandFloat(-Pi, Pi));
}
Vec3 Im3d::RandVec3(float _min, float _max)
{
	return Im3d::Vec3(
		RandFloat(_min, _max),
		RandFloat(_min, _max),
		RandFloat(_min, _max)
		);
}
Color Im3d::RandColor(float _min, float _max)
{
	Vec3 v = RandVec3(_min, _max);
	return Color(v.x, v.y, v.z);
}

/******************************************************************************/
#if defined(IM3D_OPENGL)
	static GLuint g_vaImGui; // vertex array object
	static GLuint g_vbImGui; // vertex buffer
	static GLuint g_ibImGui; // index buffer
	static GLuint g_shImGui; // shader
	static GLuint g_txImGui; // font texture

	static void ImGui_Draw(ImDrawData* _drawData)
	{
		ImGuiIO& io = ImGui::GetIO();
	
		int fbX, fbY;
		fbX = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
		fbY = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
		if (fbX == 0  || fbY == 0) {
			return;
		}
		_drawData->ScaleClipRects(io.DisplayFramebufferScale);
	
		glAssert(glViewport(0, 0, (GLsizei)fbX, (GLsizei)fbY));
		glAssert(glEnable(GL_BLEND));
		glAssert(glBlendEquation(GL_FUNC_ADD));
		glAssert(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		glAssert(glDisable(GL_CULL_FACE));
		glAssert(glDisable(GL_DEPTH_TEST));
		glAssert(glEnable(GL_SCISSOR_TEST));
		glAssert(glActiveTexture(GL_TEXTURE0));
		
		Mat4 ortho = Mat4(
			2.0f/io.DisplaySize.x, 0.0f,                   0.0f, -1.0f,
			0.0f,                  2.0f/-io.DisplaySize.y, 0.0f,  1.0f,
			0.0f,                  0.0f,                  -1.0f,  0.0f
			);
		glAssert(glUseProgram(g_shImGui));
	
		bool transpose = false;
		#ifdef IM3D_MATRIX_ROW_MAJOR
			transpose = true;
		#endif
		glAssert(glUniformMatrix4fv(glGetUniformLocation(g_shImGui, "uProjMatrix"), 1, transpose, (const GLfloat*)ortho));
		glAssert(glBindVertexArray(g_vaImGui));
	
		for (int i = 0; i < _drawData->CmdListsCount; ++i) {
			const ImDrawList* drawList = _drawData->CmdLists[i];
			const ImDrawIdx* indexOffset = 0;
	
			glAssert(glBindBuffer(GL_ARRAY_BUFFER, g_vbImGui));
			glAssert(glBufferData(GL_ARRAY_BUFFER, drawList->VtxBuffer.size() * sizeof(ImDrawVert), (GLvoid*)&drawList->VtxBuffer.front(), GL_STREAM_DRAW));
			glAssert(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibImGui));
			glAssert(glBufferData(GL_ELEMENT_ARRAY_BUFFER, drawList->IdxBuffer.Size * sizeof(ImDrawIdx), (GLvoid*)drawList->IdxBuffer.Data, GL_STREAM_DRAW));
	
			for (const ImDrawCmd* pcmd = drawList->CmdBuffer.begin(); pcmd != drawList->CmdBuffer.end(); ++pcmd) {
				if (pcmd->UserCallback) {
					pcmd->UserCallback(drawList, pcmd);
				} else {
					glAssert(glBindTexture(GL_TEXTURE_2D, (GLuint)pcmd->TextureId));
					glAssert(glScissor((int)pcmd->ClipRect.x, (int)(fbY - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y)));
					glAssert(glDrawElements(GL_TRIANGLES, pcmd->ElemCount, GL_UNSIGNED_SHORT, (GLvoid*)indexOffset));
				}
				indexOffset += pcmd->ElemCount;
			}
		}
	
		glAssert(glDisable(GL_SCISSOR_TEST));
		glAssert(glDisable(GL_BLEND));
		glAssert(glUseProgram(0));
	}

	static bool ImGui_Init()
	{
		GLuint vs = LoadCompileShader(GL_VERTEX_SHADER,   "imgui.glsl", "VERTEX_SHADER\0");
		GLuint fs = LoadCompileShader(GL_FRAGMENT_SHADER, "imgui.glsl", "FRAGMENT_SHADER\0");
		if (vs && fs) {
			glAssert(g_shImGui = glCreateProgram());
			glAssert(glAttachShader(g_shImGui, vs));
			glAssert(glAttachShader(g_shImGui, fs));
			bool ret = LinkShaderProgram(g_shImGui);
			glAssert(glDeleteShader(vs));
			glAssert(glDeleteShader(fs));
			if (!ret) {
				return false;
			}
		} else {
			return false;
		}
		glAssert(glUseProgram(g_shImGui));
		glAssert(glUniform1i(glGetUniformLocation(g_shImGui, "txTexture"), 0));
		glAssert(glUseProgram(0));

		glAssert(glCreateBuffers(1, &g_vbImGui));
		glAssert(glCreateBuffers(1, &g_ibImGui));
		glAssert(glCreateVertexArrays(1, &g_vaImGui));	
		glAssert(glBindVertexArray(g_vaImGui));
		glAssert(glBindBuffer(GL_ARRAY_BUFFER, g_vbImGui));
		glAssert(glEnableVertexAttribArray(0));
		glAssert(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, pos)));
		glAssert(glEnableVertexAttribArray(1));
		glAssert(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, uv)));
		glAssert(glEnableVertexAttribArray(2));
		glAssert(glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, col)));
		glAssert(glBindVertexArray(0));
	
		unsigned char* txbuf;
		int txX, txY;
		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->GetTexDataAsAlpha8(&txbuf, &txX, &txY);
		glAssert(glGenTextures(1, &g_txImGui));
		glAssert(glBindTexture(GL_TEXTURE_2D, g_txImGui));
		glAssert(glTextureParameteri(g_txImGui, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		glAssert(glTextureParameteri(g_txImGui, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		glAssert(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, txX, txY, 0, GL_RED, GL_UNSIGNED_BYTE, (const GLvoid*)txbuf));
		io.Fonts->TexID = (void*)g_txImGui;
	
		io.RenderDrawListsFn = &ImGui_Draw;

		return true;

	}

	static void ImGui_Shutdown()
	{
		glAssert(glDeleteVertexArrays(1, &g_vaImGui));
		glAssert(glDeleteBuffers(1, &g_vbImGui));
		glAssert(glDeleteBuffers(1, &g_ibImGui));		
		glAssert(glDeleteProgram(g_shImGui));
		glAssert(glDeleteTextures(1, &g_txImGui));
	}
#endif

#if defined(IM3D_PLATFORM_WIN)
	static void ImGui_Update()
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeyMap[ImGuiKey_Tab]        = VK_TAB;
		io.KeyMap[ImGuiKey_LeftArrow]  = VK_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow]	   = VK_UP;
		io.KeyMap[ImGuiKey_DownArrow]  = VK_DOWN;
		io.KeyMap[ImGuiKey_PageUp]	   = VK_PRIOR;
		io.KeyMap[ImGuiKey_PageDown]   = VK_NEXT;
		io.KeyMap[ImGuiKey_Home]	   = VK_HOME;
		io.KeyMap[ImGuiKey_End]		   = VK_END;
		io.KeyMap[ImGuiKey_Delete]	   = VK_DELETE;
		io.KeyMap[ImGuiKey_Backspace]  = VK_BACK;
		io.KeyMap[ImGuiKey_Enter]	   = VK_RETURN;
		io.KeyMap[ImGuiKey_Escape]	   = VK_ESCAPE;
		io.KeyMap[ImGuiKey_A]		   = 0x41;
		io.KeyMap[ImGuiKey_C]		   = 0x43;
		io.KeyMap[ImGuiKey_V]		   = 0x56;
		io.KeyMap[ImGuiKey_X]		   = 0x58;
		io.KeyMap[ImGuiKey_Y]		   = 0x59;
		io.KeyMap[ImGuiKey_Z]		   = 0x5A;

		io.ImeWindowHandle = g_Example->m_hwnd;
		io.DisplaySize = ImVec2((float)g_Example->m_width, (float)g_Example->m_height);
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
		io.DeltaTime = g_Example->m_deltaTime;

		ImGui::NewFrame();
	}
#endif

/******************************************************************************/
Example* Im3d::g_Example;

bool Example::init(int _width, int _height, const char* _title)
{
	g_Example = this;

	#if defined(IM3D_PLATFORM_WIN)
	 // force the current working directory to the exe location
		TCHAR buf[MAX_PATH] = {};
		DWORD buflen;
		winAssert(buflen = GetModuleFileName(0, buf, MAX_PATH));
		char* pathend = strrchr(buf, (int)'\\');
		*(++pathend) = '\0';
		winAssert(SetCurrentDirectory(buf));
		fprintf(stdout, "Set current directory: '%s'\n", buf);
		
		winAssert(QueryPerformanceFrequency(&g_SysTimerFreq));
		winAssert(QueryPerformanceCounter(&m_currTime));
	#endif

	m_width  = _width;
	m_height = _height;
	m_title  = _title;
	if (!InitWindow(m_width, m_height, m_title)) {
		goto Example_init_fail;
	}
	#if defined(IM3D_OPENGL) 
		if (!InitOpenGL(IM3D_OPENGL_VMAJ, IM3D_OPENGL_VMIN)) {
			goto Example_init_fail;
		}
	#endif

	if (!ImGui_Init()) {
		goto Example_init_fail;
	}	
	if (!Im3d_Init()) {	
		goto Example_init_fail;
	}

	m_camPos = Vec3(0.0f, 2.0f, 3.0f);
	m_camDir = Normalize(Vec3(0.0f, -0.5f, -1.0f));
	m_camFovDeg = 50.0f;
	
	return true;

Example_init_fail:
	shutdown();
	return false;
}

void Example::shutdown()
{
	ImGui_Shutdown();
	Im3d_Shutdown();

	#if defined(IM3D_OPENGL) 
		ShutdownOpenGL();
	#endif
	
	ShutdownWindow();
}

bool Example::update()
{
	bool ret = true;
	#if defined(IM3D_PLATFORM_WIN)
		g_Example->m_prevTime = g_Example->m_currTime;
		winAssert(QueryPerformanceCounter(&m_currTime));
		double microseconds = (double)((g_Example->m_currTime.QuadPart - g_Example->m_prevTime.QuadPart) * 1000000ll / g_SysTimerFreq.QuadPart);
		m_deltaTime = (float)(microseconds / 1000000.0);
	
		MSG msg;
		while (PeekMessage(&msg, g_Example->m_hwnd, 0, 0, PM_REMOVE) && msg.message != WM_QUIT) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		ret = msg.message != WM_QUIT;
	#endif
		
	ImGui_Update();



	float kCamSpeed = 2.0f;
	float kCamSpeedMul = 10.0f;
	float kCamRotationMul = 0.8f;
	m_camWorld = LookAt(m_camPos, m_camPos - m_camDir);
	m_camView = Inverse(m_camWorld);
	#if defined(IM3D_PLATFORM_WIN)
		Vec2 cursorPos = getWindowRelativeCursor();
		if (hasFocus()) {
			if (!ImGui::GetIO().WantCaptureKeyboard) {
				if (GetAsyncKeyState(VK_LSHIFT) & 0x8000) {
					kCamSpeed *= 10.0f;
				}
				if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) == 0 ) { // ctrl not pressed
					if (GetAsyncKeyState(0x57) & 0x8000) { // W (forward)
						m_camPos = m_camPos - m_camWorld.getCol(2) * (m_deltaTime * kCamSpeed);
					}
					if (GetAsyncKeyState(0x41) & 0x8000) { // A (left)
						m_camPos = m_camPos - m_camWorld.getCol(0) * (m_deltaTime * kCamSpeed);
					}
					if (GetAsyncKeyState(0x53) & 0x8000) { // S (backward)
						m_camPos = m_camPos + m_camWorld.getCol(2) * (m_deltaTime * kCamSpeed);
					}
					if (GetAsyncKeyState(0x44) & 0x8000) { // D (right)
						m_camPos = m_camPos + m_camWorld.getCol(0) * (m_deltaTime * kCamSpeed);
					}
					if (GetAsyncKeyState(0x51) & 0x8000) { // Q (down)
						m_camPos = m_camPos - m_camWorld.getCol(1)* (m_deltaTime * kCamSpeed);
					}
					if (GetAsyncKeyState(0x45) & 0x8000) { // D (up)
						m_camPos = m_camPos + m_camWorld.getCol(1) * (m_deltaTime * kCamSpeed);
					}
				}
			}
			if (!ImGui::GetIO().WantCaptureMouse) {
				if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) {
					Vec2 cursorDelta = (cursorPos - m_prevCursorPos) * m_deltaTime * kCamRotationMul;
					m_camDir = Rotation(Vec3(0.0f, 1.0f, 0.0f), -cursorDelta.x) * m_camDir;
					m_camDir = Rotation(m_camWorld.getCol(0), -cursorDelta.y) * m_camDir;
				}
			}
		}
		m_prevCursorPos = cursorPos;
	#endif

	m_camFovRad = Im3d::Radians(m_camFovDeg);
	float n = 0.1f;
	float f = 10000.0f;
	float a = (float)m_width / (float)m_height;
	float scale = tanf(m_camFovRad * 0.5f) * n;
	float r = a * scale;
	float l = -r;
	float t = scale;
	float b = -t;

	m_camProj = Mat4(
		2.0f * n / (r - l),  0.0f,                 (r + l) / (r - l),   0.0f,
		0.0f,                2.0f * n / (t - b),   (t + b) / (t - b),   0.0f,
		0.0f,                0.0f,                -(f + n) / (f - n),  -2.0f * f * n / (f - n),
		0.0f,                0.0f,                -1.0f,                0.0f
		);

	m_camWorld = LookAt(m_camPos, m_camPos - m_camDir);
	m_camView  = Inverse(m_camWorld);
	m_camViewProj = m_camProj * m_camView;

	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::Begin(
		"Frame Info", 0, 
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_AlwaysAutoResize
		);
		ImGui::Text("%.2f fps", 1.0f / m_deltaTime);
		ImGui::Text("Triangles: %u ", Im3d::GetContext().getPrimitiveCount(Im3d::DrawPrimitive_Triangles));
		ImGui::Text("Lines:     %u ", Im3d::GetContext().getPrimitiveCount(Im3d::DrawPrimitive_Lines));
		ImGui::Text("Points:    %u ", Im3d::GetContext().getPrimitiveCount(Im3d::DrawPrimitive_Points));
	ImGui::End();

	Im3d_Update();
	
	return ret;
}

void Example::draw()
{
	Im3d::Draw();
	ImGui::Render();

	#if defined(IM3D_PLATFORM_WIN)
		#if defined(IM3D_OPENGL)
			winAssert(SwapBuffers(m_hdc));
			winAssert(ValidateRect(m_hwnd, 0)); // suppress WM_PAINT
		#endif
	#endif
	
 // reset state & clear backbuffer for next frame
	#if defined(IM3D_OPENGL)
		glAssert(glBindVertexArray(0));
		glAssert(glUseProgram(0));
		glAssert(glViewport(0, 0, m_width, m_height));
		glAssert(glClearColor(0.5f, 0.5f, 0.5f, 0.0f));
		glAssert(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	#endif
}

bool Example::hasFocus() const
{
	#if defined(IM3D_PLATFORM_WIN)
		return m_hwnd == GetFocus();
	#endif
}

Vec2 Example::getWindowRelativeCursor() const
{
	#if defined(IM3D_PLATFORM_WIN)
		POINT p = {};
		winAssert(GetCursorPos(&p));
		winAssert(ScreenToClient(m_hwnd, &p));
		return Vec2((float)p.x, (float)p.y);
	#endif
}
