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
	if (!app.init(-1, -1, "Im3d Tests")) {
		return 1;
	}
	while (app.update()) {
		glAssert(glViewport(0, 0, app.getWidth(), app.getHeight()));
		glAssert(glClearColor(0.25f, 0.25f, 0.25f, 1.0f));
		glAssert(glClear(GL_COLOR_BUFFER_BIT));
		
		static const int   kGridSize = 20;
		static const float kGridHalf = (float)kGridSize * 0.5f;
		Im3d::PushDrawState();
			Im3d::SetAlpha(1.0f);
			Im3d::SetSize(1.0f);
			Im3d::BeginLines();
				for (int x = 0; x <= kGridSize; ++x) {
					Im3d::Vertex(-kGridHalf, 0.0f, (float)x - kGridHalf,  Im3d::Color(0.0f, 0.0f, 0.0f));
					Im3d::Vertex( kGridHalf, 0.0f, (float)x - kGridHalf,  Im3d::Color(1.0f, 0.0f, 0.0f));
				}
				for (int z = 0; z <= kGridSize; ++z) {
					Im3d::Vertex((float)z - kGridHalf, 0.0f, -kGridHalf,  Im3d::Color(0.0f, 0.0f, 0.0f));
					Im3d::Vertex((float)z - kGridHalf, 0.0f,  kGridHalf,  Im3d::Color(0.0f, 0.0f, 1.0f));
				}
			Im3d::End();

			Im3d::SetSize(8.0f);
			Im3d::BeginPoints();
				RandSeed(123);
				for (int i = 0; i < 200; ++i) {
					Im3d::SetColor(RandFloat(0.5f, 1.0f), RandFloat(0.5f, 1.0f), RandFloat(0.5f, 1.0f));
					Im3d::Vertex(RandVec3(-10.0f, 10.0f));
				}
			Im3d::End();

			Im3d::SetSize(1.0f);
			Im3d::BeginTriangles();
				Im3d::Vertex( 1.0f, -1.0f, -5.0f, Color_Blue);
				Im3d::Vertex( 0.0f,  1.0f, -5.0f, Color_Red);
				Im3d::Vertex(-1.0f, -1.0f, -5.0f, Color_Green);
			Im3d::End();

			Im3d::SetSize(2.0f);
			Im3d::BeginLineLoop();
				Im3d::SetColor(Color_Cyan);
				Im3d::Vertex( 1.0f, -1.0f, -5.0f);
				Im3d::SetColor(Color_Magenta);
				Im3d::Vertex( 0.0f,  1.0f, -5.0f);
				Im3d::SetColor(Color_Yellow);
				Im3d::Vertex(-1.0f, -1.0f, -5.0f);
			Im3d::End();

		Im3d::PopDrawState();

		app.draw();
	}
	app.shutdown();

	return 0;
}
