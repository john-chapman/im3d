#include "TestApp.h"

#include <cstring>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

using namespace Im3d;

struct TestApp::Impl
{
 // window
	HWND   m_hwnd;

	static LRESULT CALLBACK WindowProc(HWND _hwnd, UINT _umsg, WPARAM _wparam, LPARAM _lparam);

 // gl context
	HDC    m_hdc;
	HGLRC  m_hglrc;

};

LRESULT CALLBACK TestApp::Impl::WindowProc(HWND _hwnd, UINT _umsg, WPARAM _wparam, LPARAM _lparam)
{
	return DefWindowProc(_hwnd, _umsg, _wparam, _lparam);
}


/*******************************************************************************

                                  TestApp

*******************************************************************************/

// PUBLIC

bool TestApp::init(int _width, int _height, const char* _title)
{
	shutdown();
	
	m_impl = new Impl;

	m_width = _width;
	m_height = _height;
	m_title = _title;

	static ATOM wndclassex = 0;
	if (wndclassex == 0) {
		WNDCLASSEX wc;
		memset(&wc, 0, sizeof(wc));
		wc.cbSize = sizeof(wc);
		wc.style = CS_OWNDC;// | CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = Impl::WindowProc;
		wc.hInstance = GetModuleHandle(0);
		wc.lpszClassName = "TestAppImpl";
		wc.hCursor = LoadCursor(0, IDC_ARROW);
		APT_PLATFORM_VERIFY(wndclassex = RegisterClassEx(&wc));
	}

	DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_MINIMIZEBOX | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	if (m_width == -1 || m_height == -1) {
	 // auto size; get the dimensions of the primary screen area and subtract the non-client area
		RECT r;
		APT_PLATFORM_VERIFY(SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0));
		_width  = r.right - r.left;
		_height = r.bottom - r.top;

		RECT wr = {};
		APT_PLATFORM_VERIFY(AdjustWindowRectEx(&wr, dwStyle, FALSE, dwExStyle));
		_width  -= wr.right - wr.left;
		_height -= wr.bottom - wr.top;
	}

	RECT r; r.top = 0; r.left = 0; r.bottom = _height; r.right = _width;
	APT_PLATFORM_VERIFY(AdjustWindowRectEx(&r, dwStyle, FALSE, dwExStyle));
	impl->m_hwnd = CreateWindowEx(
		dwExStyle, 
		MAKEINTATOM(wndclassex), 
		ret->m_title, 
		dwStyle, 
		0, 0, 
		r.right - r.left, r.bottom - r.top, 
		NULL, 
		NULL, 
		GetModuleHandle(0), 
		NULL
		);
	APT_PLATFORM_ASSERT(impl->m_hwnd);
	ShowWindow(impl->m_hwnd, SW_SHOW);
	return true;
}

void TestApp::shutdown()
{
	if (m_impl) {

		APT_PLATFORM_VERIFY(DestroyWindow(m_impl->m_handle));
		delete m_impl;
	}
}