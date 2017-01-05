#include "common.h"

#include <cstdarg>
#include <cstdio>

#ifdef IM3D_COMPILER_MSVC
	#pragma warning(disable: 4996) // vsnprintf
#endif

using namespace Im3d;

static const char* StripPath(const char* _path) 
{
	int i = 0, last = 0;
	while (_path[i] != 0) {
		if (_path[i] == '\\' || _path[i] == '/')
			last = i + 1;
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

const char* Im3d::GetPlatformErrorString(void* _err)
{
	const int kErrMsgMax = 1024;
	
	static char buf[kErrMsgMax];
	buf[0] = '\0';
	IM3D_VERIFY(
		FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
			NULL, 
			(DWORD)_err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)buf, 
			kErrMsgMax, 
			NULL
		) != 0
	);
	return buf;
}