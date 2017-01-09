#include "TestApp.h"

#include "common.h"

#include "GL/wglew.h"
#include <cstring>
#include <cstdio>

using namespace Im3d;

static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormat = 0;
static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribs = 0;

static const char* kGlslVersionString = "#version 450";

// \todo make everything static?
static TestApp* s_testApp = 0;

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

// Returns 0 on failure, _defines is a list of null-separated strings e.g. "DEFINE1 1\0DEFINE2 1\0"
static GLuint LoadCompileShader(GLenum _stage, const char* _path, const char* _defines = 0)
{
	Vector<char> src;
	AppendLine(kGlslVersionString, src);
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
	IM3D_VERIFY(fseek(fin, 0, SEEK_END) == 0); // not portable but works on almost all implementations
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

		fprintf(stderr, "\n\n%s", src.data());
		fprintf(stderr, "\n");
		glAssert(glDeleteShader(ret));
		return 0;
	}
	return ret;
}

static bool LinkShaderProgram(GLuint _handle)
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

/*******************************************************************************

                                   ImGui

*******************************************************************************/
static GLuint  s_vaImGui; // vertex array object
static GLuint  s_vbImGui; // vertex buffer
static GLuint  s_ibImGui; // index buffer
static GLuint  s_shImGui;
static GLuint  s_txImGui;

static bool ImGui_Init();
static void ImGui_Draw(ImDrawData*);

static bool ImGui_Init()
{
	GLuint vs = LoadCompileShader(GL_VERTEX_SHADER,   "imgui.glsl", "VERTEX_SHADER\0");
	GLuint fs = LoadCompileShader(GL_FRAGMENT_SHADER, "imgui.glsl", "FRAGMENT_SHADER\0");
	if (vs && fs) {
		glAssert(s_shImGui = glCreateProgram());
		glAssert(glAttachShader(s_shImGui, vs));
		glAssert(glAttachShader(s_shImGui, fs));
		LinkShaderProgram(s_shImGui);
	}
	ImGuiIO& io = ImGui::GetIO();
	glAssert(glCreateBuffers(1, &s_vbImGui));
	glAssert(glCreateBuffers(1, &s_ibImGui));
	glAssert(glCreateVertexArrays(1, &s_vaImGui));	
	glAssert(glBindVertexArray(s_vaImGui));
	glAssert(glBindBuffer(GL_ARRAY_BUFFER, s_vbImGui));
	glAssert(glEnableVertexAttribArray(0));
    glAssert(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, pos)));
	glAssert(glEnableVertexAttribArray(1));
    glAssert(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, uv)));
	glAssert(glEnableVertexAttribArray(2));
    glAssert(glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, col)));
	glAssert(glBindVertexArray(0));

	unsigned char* txbuf;
	int txX, txY;
	io.Fonts->GetTexDataAsAlpha8(&txbuf, &txX, &txY);
	glAssert(glGenTextures(1, &s_txImGui));
	glAssert(glBindTexture(GL_TEXTURE_2D, s_txImGui));
	glAssert(glTextureParameteri(s_txImGui, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	glAssert(glTextureParameteri(s_txImGui, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	glAssert(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, txX, txY, 0, GL_RED, GL_UNSIGNED_BYTE, (const GLvoid*)txbuf));
	io.Fonts->TexID = (void*)s_txImGui;

	io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
	io.RenderDrawListsFn = &ImGui_Draw;

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
	return true;
}

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
		0.0f,                  0.0f,                  -1.0f,  0.0f,
		0.0f,                  0.0f,                   0.0f,  1.0f
		);
	glAssert(glUseProgram(s_shImGui));
	glAssert(glUniformMatrix4fv(glGetUniformLocation(s_shImGui, "uProjMatrix"), 1, false, (const GLfloat*)ortho));
	glAssert(glBindVertexArray(s_vaImGui));

	for (int i = 0; i < _drawData->CmdListsCount; ++i) {
		const ImDrawList* drawList = _drawData->CmdLists[i];
		const ImDrawIdx* indexOffset = 0;

	 // upload vertex/index data
		glAssert(glBindBuffer(GL_ARRAY_BUFFER, s_vbImGui));
		glAssert(glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)drawList->VtxBuffer.size() * sizeof(ImDrawVert), (GLvoid*)&drawList->VtxBuffer.front(), GL_STREAM_DRAW));
		glAssert(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_ibImGui));
		glAssert(glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)drawList->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)drawList->IdxBuffer.Data, GL_STREAM_DRAW));

	 // dispatch draw commands
		for (const ImDrawCmd* pcmd = drawList->CmdBuffer.begin(); pcmd != drawList->CmdBuffer.end(); ++pcmd) {
			if (pcmd->UserCallback) {
				pcmd->UserCallback(drawList, pcmd);
			} else {
				glAssert(glBindTexture(GL_TEXTURE_2D, (GLuint)pcmd->TextureId));
                glAssert(glScissor((int)pcmd->ClipRect.x, (int)(fbY - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y)));
				glAssert(glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, GL_UNSIGNED_SHORT, (GLvoid*)indexOffset));
			}
			indexOffset += pcmd->ElemCount;
		}
	}

	glAssert(glDisable(GL_SCISSOR_TEST));
	glAssert(glDisable(GL_BLEND));
	glAssert(glUseProgram(0));
}

/*******************************************************************************

                                    Im3d

*******************************************************************************/
static GLuint  s_vaIm3d;           // vertex array object
static GLuint  s_vbIm3d;           // vertex buffer
static GLuint  s_shIm3dPoints;
static GLuint  s_shIm3dLines;
static GLuint  s_shIm3dTriangles;

void Im3d_Draw(Im3d::DrawPrimitiveType _primType, const Im3d::VertexData* _data, Im3d::U32 _count);

static bool Im3d_Init()
{
	{
		GLuint vs = LoadCompileShader(GL_VERTEX_SHADER,   "im3d.glsl", "VERTEX_SHADER\0POINTS\0");
		GLuint fs = LoadCompileShader(GL_FRAGMENT_SHADER, "im3d.glsl", "FRAGMENT_SHADER\0POINTS\0");
		if (vs && fs) {
			glAssert(s_shIm3dPoints = glCreateProgram());
			glAssert(glAttachShader(s_shIm3dPoints, vs));
			glAssert(glAttachShader(s_shIm3dPoints, fs));
			if (!LinkShaderProgram(s_shIm3dPoints)) {
				return false;
			}
		} else {
			return false;
		}
	}
	{
		GLuint vs = LoadCompileShader(GL_VERTEX_SHADER,   "im3d.glsl", "VERTEX_SHADER\0LINES\0");
		GLuint gs = LoadCompileShader(GL_GEOMETRY_SHADER, "im3d.glsl", "GEOMETRY_SHADER\0LINES\0");
		GLuint fs = LoadCompileShader(GL_FRAGMENT_SHADER, "im3d.glsl", "FRAGMENT_SHADER\0LINES\0");
		if (vs && gs && fs) {
			glAssert(s_shIm3dLines = glCreateProgram());
			glAssert(glAttachShader(s_shIm3dLines, vs));
			glAssert(glAttachShader(s_shIm3dLines, gs));
			glAssert(glAttachShader(s_shIm3dLines, fs));
			if (!LinkShaderProgram(s_shIm3dLines)) {
				return false;
			}
		} else {
			return false;
		}
	}
	{
		GLuint vs = LoadCompileShader(GL_VERTEX_SHADER,   "im3d.glsl", "VERTEX_SHADER\0TRIANGLES\0");
		GLuint fs = LoadCompileShader(GL_FRAGMENT_SHADER, "im3d.glsl", "FRAGMENT_SHADER\0TRIANGLES\0");
		if (vs && fs) {
			glAssert(s_shIm3dTriangles = glCreateProgram());
			glAssert(glAttachShader(s_shIm3dTriangles, vs));
			glAssert(glAttachShader(s_shIm3dTriangles, fs));
			if (!LinkShaderProgram(s_shIm3dTriangles)) {
				return false;
			}
		} else {
			return false;
		}
	}

	glAssert(glCreateBuffers(1, &s_vbIm3d));;
	glAssert(glCreateVertexArrays(1, &s_vaIm3d));	
	glAssert(glBindVertexArray(s_vaIm3d));
	glAssert(glBindBuffer(GL_ARRAY_BUFFER, s_vbIm3d));
	glAssert(glEnableVertexAttribArray(0));
    glAssert(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Im3d::VertexData), (GLvoid*)offsetof(Im3d::VertexData, m_positionSize)));
	glAssert(glEnableVertexAttribArray(1));
    glAssert(glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Im3d::VertexData), (GLvoid*)offsetof(Im3d::VertexData, m_color)));
	glAssert(glBindVertexArray(0));


	GetAppData().drawPrimitives = &Im3d_Draw;
	
	return true;
}

static void Im3d_Draw(Im3d::DrawPrimitiveType _primType, const Im3d::VertexData* _data, Im3d::U32 _count)
{
	glAssert(glEnable(GL_BLEND));
	glAssert(glBlendEquation(GL_FUNC_ADD));
	glAssert(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	glAssert(glEnable(GL_PROGRAM_POINT_SIZE));
	glAssert(glDisable(GL_CULL_FACE));

	GLenum prim;
	GLuint sh;
	switch (_primType) {
	case Im3d::DrawPrimitive_Points:
		prim = GL_POINTS;
		sh = s_shIm3dPoints;
		ImGui::Text("Points (%d)", _count);
		break;
	case Im3d::DrawPrimitive_Lines:
		prim = GL_LINES;
		sh = s_shIm3dLines;
		ImGui::Text("Lines (%d)", _count / 2);
		break;
	case Im3d::DrawPrimitive_Triangles:
		prim = GL_TRIANGLES;
		sh = s_shIm3dTriangles;
		ImGui::Text("Tris (%d)", _count / 3);
		break;
	default:
		IM3D_ASSERT(false);
		return;
	};

	glAssert(glBindVertexArray(s_vaIm3d));
	glAssert(glBindBuffer(GL_ARRAY_BUFFER, s_vbIm3d));
	glAssert(glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)_count * sizeof(Im3d::VertexData), (GLvoid*)_data, GL_STREAM_DRAW));
	
	glAssert(glUseProgram(sh));
	glAssert(glUniformMatrix4fv(glGetUniformLocation(sh, "uViewProjMatrix"), 1, false, (const GLfloat*)s_testApp->m_camViewProj));
	glAssert(glUniform2f(glGetUniformLocation(sh, "uViewport"), (float)s_testApp->getWidth(), (float)s_testApp->getHeight()));
	glAssert(glDrawArrays(prim, 0, (GLsizei)_count));

	glAssert(glDisable(GL_PROGRAM_POINT_SIZE));
}

/*******************************************************************************

                                  TestApp

*******************************************************************************/

struct TestApp::Impl
{
 // window
	HWND   m_hwnd;

	static LRESULT CALLBACK WindowProc(HWND _hwnd, UINT _umsg, WPARAM _wparam, LPARAM _lparam);

	bool initWindow(int& _width_, int& _height_, const char* _title);

 // time
	LARGE_INTEGER m_currTime, m_prevTime;
	LARGE_INTEGER m_sysFreq;
	float m_deltaTime; // seconds

 // gl context
	HDC     m_hdc;
	HGLRC   m_hglrc;

	bool initGl(int _vmaj, int _vmin);
};

LRESULT CALLBACK TestApp::Impl::WindowProc(HWND _hwnd, UINT _umsg, WPARAM _wparam, LPARAM _lparam)
{
	ImGuiIO& io = ImGui::GetIO();

	switch (_umsg) {
	case WM_SIZE: {
		int w = (int)LOWORD(_lparam), h = (int)HIWORD(_lparam);
		if (s_testApp->m_width != w || s_testApp->m_height != h) {
			s_testApp->m_width = w;
			s_testApp->m_height = h;
		}
		break;
	}
	case WM_SIZING: {
		RECT* r = (RECT*)_lparam;
		int w = (int)(r->right - r->left);
		int h = (int)(r->bottom - r->top);
		if (s_testApp->m_width != w || s_testApp->m_height != h) {
			s_testApp->m_width = w;
			s_testApp->m_height = h;
		}
		break;
	}
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		io.MouseDown[0] = _umsg == WM_LBUTTONDOWN;
		break;
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
		io.MouseDown[2] = _umsg == WM_MBUTTONDOWN;
		break;
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		io.MouseDown[1] = _umsg == WM_RBUTTONDOWN;
		break;
	case WM_MOUSEWHEEL:
		io.MouseWheel = (float)(GET_WHEEL_DELTA_WPARAM(_wparam)) / (float)(WHEEL_DELTA); 
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
			io.KeyCtrl = _umsg == WM_KEYDOWN;
			break;
		case VK_MENU:
			io.KeyAlt = _umsg == WM_KEYDOWN;
			break;
		case VK_LSHIFT:
		case VK_RSHIFT:
			io.KeyShift = _umsg == WM_KEYDOWN;
			break;
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		default:
			if (vk < 512) {
				io.KeysDown[vk] = _umsg == WM_KEYDOWN;
			}
			break;
		};
		break;
	}
   case WM_CHAR:
		if (_wparam > 0 && _wparam < 0x10000) {
			io.AddInputCharacter((unsigned short)_wparam);
		}
		break;
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

bool TestApp::Impl::initWindow(int& _width_, int& _height_, const char* _title)
{
	static ATOM wndclassex = 0;
	if (wndclassex == 0) {
		WNDCLASSEX wc;
		memset(&wc, 0, sizeof(wc));
		wc.cbSize = sizeof(wc);
		wc.style = CS_OWNDC;// | CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = Impl::WindowProc;
		wc.hInstance = GetModuleHandle(0);
		wc.lpszClassName = "Im3dTestApp";
		wc.hCursor = LoadCursor(0, IDC_ARROW);
		IM3D_PLATFORM_VERIFY(wndclassex = RegisterClassEx(&wc));
	}

	DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_MINIMIZEBOX | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	if (_width_ == -1 || _height_ == -1) {
	 // auto size; get the dimensions of the primary screen area and subtract the non-client area
		RECT r;
		IM3D_PLATFORM_VERIFY(SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0));
		_width_  = r.right - r.left;
		_height_ = r.bottom - r.top;

		RECT wr = {};
		IM3D_PLATFORM_VERIFY(AdjustWindowRectEx(&wr, dwStyle, FALSE, dwExStyle));
		_width_  -= wr.right - wr.left;
		_height_ -= wr.bottom - wr.top;
	}

	RECT r; r.top = 0; r.left = 0; r.bottom = _height_; r.right = _width_;
	IM3D_PLATFORM_VERIFY(AdjustWindowRectEx(&r, dwStyle, FALSE, dwExStyle));
	m_hwnd = CreateWindowEx(
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
	IM3D_ASSERT(m_hwnd);
	return true;
}

bool TestApp::Impl::initGl(int _vmaj, int _vmin)
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
		IM3D_PLATFORM_VERIFY(wndclassex = RegisterClassEx(&wc));
	}
	HWND hwndDummy = CreateWindowEx(0, MAKEINTATOM(wndclassex), 0, NULL, 0, 0, 1, 1, NULL, NULL, GetModuleHandle(0), NULL);
	IM3D_PLATFORM_ASSERT(hwndDummy);
	HDC hdcDummy = 0;
	IM3D_PLATFORM_VERIFY(hdcDummy = GetDC(hwndDummy));	
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(pfd));
	IM3D_PLATFORM_VERIFY(SetPixelFormat(hdcDummy, 1, &pfd));
	HGLRC hglrcDummy = 0;
	IM3D_PLATFORM_VERIFY(hglrcDummy = wglCreateContext(hdcDummy));
	IM3D_PLATFORM_VERIFY(wglMakeCurrent(hdcDummy, hglrcDummy));
	
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
	IM3D_PLATFORM_VERIFY(wglChoosePixelFormat = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB"));
	IM3D_PLATFORM_VERIFY(wglCreateContextAttribs = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB"));

 // delete the dummy context
	IM3D_PLATFORM_VERIFY(wglMakeCurrent(0, 0));
	IM3D_PLATFORM_VERIFY(wglDeleteContext(hglrcDummy));
	IM3D_PLATFORM_VERIFY(ReleaseDC(hwndDummy, hdcDummy) != 0);
	IM3D_PLATFORM_VERIFY(DestroyWindow(hwndDummy) != 0);

 // create true context
	IM3D_PLATFORM_VERIFY(m_hdc = GetDC(m_hwnd));
	const int pfattr[] = {
		WGL_SUPPORT_OPENGL_ARB, 1,
		WGL_DRAW_TO_WINDOW_ARB, 1,
		WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
		WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
		WGL_DOUBLE_BUFFER_ARB,  1,
		WGL_COLOR_BITS_ARB,     24,
		WGL_ALPHA_BITS_ARB,     8,
		WGL_DEPTH_BITS_ARB,     0,
		WGL_STENCIL_BITS_ARB,   0,
		0
	};
    int pformat = -1, npformat = -1;
	IM3D_PLATFORM_VERIFY(wglChoosePixelFormat(m_hdc, pfattr, 0, 1, &pformat, (::UINT*)&npformat));
	IM3D_PLATFORM_VERIFY(SetPixelFormat(m_hdc, pformat, &pfd));
	int profileBit = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
	//profileBit = WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
	int attr[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB,	_vmaj,
		WGL_CONTEXT_MINOR_VERSION_ARB,	_vmin,
		WGL_CONTEXT_PROFILE_MASK_ARB,	profileBit,
		0
	};
	IM3D_PLATFORM_VERIFY(m_hglrc = wglCreateContextAttribs(m_hdc, 0, attr));

 // load extensions
	if (!wglMakeCurrent(m_hdc, m_hglrc)) {
		fprintf(stderr, "wglMakeCurrent failed");
		return false;
	}
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	IM3D_ASSERT(err == GLEW_OK);
	glGetError(); // clear any errors caused by glewInit()

	fprintf(stdout, "OpenGL context:\n\tVersion: %s\n\tGLSL Version: %s\n\tVendor: %s\n\tRenderer: %s\n",
		GlGetString(GL_VERSION),
		GlGetString(GL_SHADING_LANGUAGE_VERSION),
		GlGetString(GL_VENDOR),
		GlGetString(GL_RENDERER)
		);

	float qdata[] = {
		-1.0f, -1.0f,
		 1.0f, -1.0f,
		-1.0f,  1.0f,
		 1.0f,  1.0f
	};
	GLuint vb;
	glAssert(glCreateBuffers(1, &vb));
	glAssert(glNamedBufferData(vb, sizeof(qdata), qdata, GL_STATIC_DRAW));

	return true;
}


// PUBLIC

TestApp::TestApp()
	: m_impl(nullptr)
{
	m_camPos = Vec3(0.0f, 0.0f, 0.0f);
	m_camDir = Normalize(Vec3(0.0f, 0.0f, -1.0f));
	m_camFov = 50.0f;
}

bool TestApp::init(int _width, int _height, const char* _title)
{
	IM3D_ASSERT(s_testApp == 0); // oops, more than 1 instance?
	s_testApp = this;

	shutdown();

 // force the current working directory to the exe location
	TCHAR buf[MAX_PATH] = {};
	DWORD buflen;
	IM3D_PLATFORM_VERIFY(buflen = GetModuleFileName(0, buf, MAX_PATH));
	char* pathend = strrchr(buf, (int)'\\');
	*(++pathend) = '\0';
	IM3D_PLATFORM_VERIFY(SetCurrentDirectory(buf));
	fprintf(stdout, "Set current directory: '%s'\n", buf);
	
	m_impl = new Impl;
	
	m_width = _width;
	m_height = _height;
	m_title = _title;
	
	if (!m_impl->initWindow(m_width, m_height, m_title)) {
		shutdown();
		return false;
	}
	if (!m_impl->initGl(4, 5)) {
		shutdown();
		return false;
	}
	if (!ImGui_Init()) {
		shutdown();
		return false;
	}
	if (!Im3d_Init()) {
		shutdown();
		return false;
	}
	Im3d::GetAppData().drawPrimitives = &Im3d_Draw;

	IM3D_PLATFORM_VERIFY(QueryPerformanceFrequency(&m_impl->m_sysFreq));
	IM3D_PLATFORM_VERIFY(QueryPerformanceCounter(&m_impl->m_currTime));

	ShowWindow(m_impl->m_hwnd, SW_SHOW);
	return true;
}

void TestApp::shutdown()
{
	if (m_impl) {
		IM3D_PLATFORM_VERIFY(wglMakeCurrent(0, 0));
		IM3D_PLATFORM_VERIFY(wglDeleteContext(m_impl->m_hglrc));
		IM3D_PLATFORM_VERIFY(ReleaseDC(m_impl->m_hwnd, m_impl->m_hdc) != 0);
		IM3D_PLATFORM_VERIFY(DestroyWindow(m_impl->m_hwnd));
		delete m_impl;
	}
}

bool TestApp::update()
{
	m_impl->m_prevTime = m_impl->m_currTime;
	IM3D_PLATFORM_VERIFY(QueryPerformanceCounter(&m_impl->m_currTime));
	double microseconds = (double)((m_impl->m_currTime.QuadPart - m_impl->m_prevTime.QuadPart) * 1000000ll / m_impl->m_sysFreq.QuadPart);
	m_impl->m_deltaTime = (float)(microseconds / 1000000.0);
	m_deltaTime = m_impl->m_deltaTime;

	MSG msg;
	while (PeekMessage(&msg, (HWND)m_impl->m_hwnd, 0, 0, PM_REMOVE) && msg.message != WM_QUIT) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	static const float kCamSpeed = 2.0f;
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

	if (GetAsyncKeyState(VK_ADD) & 0x8000) { // + (rotate right)
		m_camDir = Rotate(Mat4(1.0f), Vec3(0.0f, 1.0f, 0.0f), -m_deltaTime) * m_camDir;
	}
	if (GetAsyncKeyState(VK_SUBTRACT) & 0x8000) { // - (rotate left)
		m_camDir = Rotate(Mat4(1.0f), Vec3(0.0f, 1.0f, 0.0f), m_deltaTime) * m_camDir;
	}


	float fovRads = m_camFov * (3.1415926f / 180.0f);
	float n = 0.1f;
	float f = 10000.0f;
	float a = (float)m_width / (float)m_height;
	float scale = tanf(fovRads * 0.5f) * n;
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
	m_camView = Inverse(m_camWorld);
	m_camViewProj = m_camProj * m_camView;
	AppData& ad = GetAppData();
	ad.m_deltaTime = m_deltaTime;
	ad.m_viewportSize = Vec2((float)m_width, (float)m_height);
	ad.m_viewOrigin = m_camPos;
	ad.m_tanHalfFov = tanf(fovRads * 0.5f);
	Im3d::NewFrame();

	ImGuiIO& io = ImGui::GetIO();
	io.ImeWindowHandle = m_impl->m_hwnd;
	io.DisplaySize = ImVec2((float)m_width, (float)m_height);
	io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
	io.DeltaTime = m_impl->m_deltaTime;
	io.MousePos = ImVec2(-1.0f, -1.0f);
	if (m_impl->m_hwnd == GetFocus()) {
		POINT p = {};
		IM3D_PLATFORM_VERIFY(GetCursorPos(&p));
		IM3D_PLATFORM_VERIFY(ScreenToClient(m_impl->m_hwnd, &p));
		ImGui::GetIO().MousePos = ImVec2((float)p.x, (float)p.y);
	}
	ImGui::NewFrame();

	ImGui::SliderFloat("Fov", &m_camFov, 1.0f, 90.0f);
	ImGui::Text("Delta t: %.2f", m_deltaTime);
	ImGui::Text("Cam pos: %.2f,%.2f,%.2f", m_camPos.x, m_camPos.y, m_camPos.z);
	ImGui::Text("Cam dir: %.2f,%.2f,%.2f", m_camDir.x, m_camDir.y, m_camDir.z);

	return msg.message != WM_QUIT;
}

void TestApp::draw()
{
	glAssert(glViewport(0, 0, m_width, m_height));

	Im3d::Draw();
	ImGui::Render();

	/*glMatrixMode(GL_PROJECTION);
	glLoadMatrixf((GLfloat*)m_camProj);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf((GLfloat*)m_camView);
	glColor3f(1.0f, 0.0f, 1.0f);
	glBegin(GL_LINES);
		glVertex3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(1.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 1.0f, 0.0f);
		glVertex3f(0.0f, -1.0f, 0.0f);
	glEnd();*/

	glAssert(glBindVertexArray(0));
	glAssert(glUseProgram(0));
	IM3D_PLATFORM_VERIFY(SwapBuffers(m_impl->m_hdc));
	IM3D_PLATFORM_VERIFY(ValidateRect(m_impl->m_hwnd, 0)); // suppress WM_PAINT
}
