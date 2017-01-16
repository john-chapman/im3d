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
		
		RandSeed(123);

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

			Im3d::AppData& ad = Im3d::GetAppData();
			Im3d::Ray ray(ad.m_cursorRayOrigin, ad.m_cursorRayDirection);
			Im3d::Plane plane(Im3d::Vec3(0.0f, 1.0f, 0.0f), Im3d::Vec3(0.0f));
			float t0, t1;
			if (Im3d::Intersect(ray, plane, t0)) {
				Im3d::SetColor(Im3d::Color_Magenta);
				Im3d::SetSize(12.0f);
				Im3d::BeginPoints();
					Im3d::Vertex(ray.m_origin + ray.m_direction * t0);
				Im3d::End();
			}
			ImGui::Text("Ray = %f,%f,%f", ad.m_cursorRayDirection.x, ad.m_cursorRayDirection.y, ad.m_cursorRayDirection.z);
			ImGui::Text("t0 = %f", t0);

			/*Im3d::SetSize(8.0f);
			Im3d::BeginPoints();
				RandSeed(123);
				for (int i = 0; i < 200; ++i) {
					Im3d::SetColor(RandFloat(0.5f, 1.0f), RandFloat(0.5f, 1.0f), RandFloat(0.5f, 1.0f));
					Im3d::Vertex(RandVec3(-10.0f, 10.0f));
				}
			Im3d::End();*/

			Im3d::SetAlpha(1.0f);
			Im3d::SetSize(2.0f);
			Im3d::DrawXyzAxes();
			Im3d::SetColor(Im3d::Color_Magenta);
			Im3d::DrawSphere(Vec3(2.0f, 0.0f, 0.0f), 1.0f);
			Im3d::SetColor(Im3d::Color_Yellow);
			Im3d::DrawCapsule(Vec3(4.0f, -1.0f, 0.0f), Vec3(4.0f, 1.0f, 0.0f), 1.0f);
			Im3d::SetColor(Im3d::Color_Blue);
			Im3d::DrawCylinder(Vec3(-2.0f, -1.0f, 0.0f), Vec3(-2.0f, 1.0f, 0.0f), 1.0f);
			Im3d::SetColor(Im3d::Color_Red);
			Im3d::DrawAlignedBox(Vec3(-5.0f, -1.0f, -2.0f), Vec3(-4.0f, 1.0f, 2.0f));

			static bool s_sorting = true;
			ImGui::Checkbox("Enable Sorting", &s_sorting);
			Im3d::EnableSorting(s_sorting);
			/*float x = -0.2f;
			float z = -4.0f;
			for (int i = 0; i < 3; ++i, z -= 2.0f, x += 0.2f){
				Im3d::SetSize(1.0f);
				Im3d::SetAlpha(0.8f);
				Im3d::BeginTriangles();
					Im3d::Vertex( 1.0f + x, -1.0f, z, Color_Blue);
					Im3d::Vertex( 0.0f + x,  1.0f, z, Color_Red);
					Im3d::Vertex(-1.0f + x, -1.0f, z, Color_Green);
				Im3d::End();

				Im3d::SetSize(2.0f);
				Im3d::SetAlpha(1.0f);
				Im3d::BeginLineLoop();
					Im3d::Vertex( 1.0f + x, -1.0f, z, Color_Cyan);
					Im3d::Vertex( 0.0f + x,  1.0f, z, Color_Magenta);
					Im3d::Vertex(-1.0f + x, -1.0f, z, Color_Green);
				Im3d::End();
			}*/

			static const int kCount = 50;
			Im3d::SetAlpha(0.7f);
			for (int i = 0; i < kCount; ++i) {
				Im3d::PushMatrix();
					Im3d::Mat4 m = Im3d::Rotate(Im3d::Mat4(1.0f), Im3d::Vec3(0.0f, 0.0f, 1.0f), RandFloat(-3.0f, 3.0f));
					m = Im3d::Rotate(m, Im3d::Vec3(1.0f, 0.0f, 0.0f), RandFloat(-3.0f, 3.0f));
					m = Im3d::Rotate(m, Im3d::Vec3(0.0f, 1.0f, 0.0f), RandFloat(-3.0f, 3.0f));
					m = Im3d::Translate(m, RandVec3(-10.0f, 10.0f));
					Im3d::SetMatrix(m);
					Im3d::BeginTriangles();
						Im3d::Vertex( 1.0f, -1.0f, 0.0f, Color_Blue);
						Im3d::Vertex( 0.0f,  1.0f, 0.0f, Color_Red);
						Im3d::Vertex(-1.0f, -1.0f, 0.0f, Color_Green);
					Im3d::End();
				Im3d::PopMatrix();

				Im3d::PushMatrix();
					m = Im3d::Rotate(Im3d::Mat4(1.0f), Im3d::Vec3(0.0f, 0.0f, 1.0f), RandFloat(-3.0f, 3.0f));
					m = Im3d::Rotate(m, Im3d::Vec3(1.0f, 0.0f, 0.0f), RandFloat(-3.0f, 3.0f));
					m = Im3d::Rotate(m, Im3d::Vec3(0.0f, 1.0f, 0.0f), RandFloat(-3.0f, 3.0f));
					m = Im3d::Translate(m, RandVec3(-10.0f, 10.0f));
					Im3d::SetMatrix(m);
					Im3d::SetSize(10.0f);
					Im3d::BeginLines();
						Im3d::Vertex( 0.0f, -1.0f, 0.0f, Color_Yellow);
						Im3d::Vertex( 0.0f,  1.0f, 0.0f, Color_Magenta);
					Im3d::End();
				Im3d::PopMatrix();
			}
		Im3d::PopDrawState();

		app.draw();
	}
	app.shutdown();

	return 0;
}
