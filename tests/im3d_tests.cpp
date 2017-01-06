#include "common.h"
#include "TestApp.h"

#include <cstdio>

using namespace Im3d;

template <typename T>
int GetSize();
	template <> int GetSize<Vec2>() { return 2;  }
	template <> int GetSize<Vec3>() { return 3;  }
	template <> int GetSize<Vec4>() { return 4;  }

void Print(const Vec2& _v)  { printf("(%f, %f)",         _v.x, _v.y);                  }
void Print(const Vec3& _v)  { printf("(%f, %f, %f)",     _v.x, _v.y, _v.z);            }
void Print(const Vec4& _v)  { printf("(%f, %f, %f, %f)", _v.x, _v.y, _v.z, _v.w);      }
void Print(Color _c)        { printf("%.8x = ", _c.v); Print(Vec4(_c)); printf("\n"); }

int main(int, char**)
{
	TestApp app;
	if (!app.init(512, 512, "Im3d Tests")) {
		return 1;
	}
	while (app.update()) {
		glAssert(glViewport(0, 0, app.getWidth(), app.getHeight()));
		glAssert(glClearColor(0.25f, 0.25f, 0.25f, 1.0f));
		glAssert(glClear(GL_COLOR_BUFFER_BIT));

		ImGui::ShowTestWindow();

		app.draw();
	}
	app.shutdown();

	return 0;
}
