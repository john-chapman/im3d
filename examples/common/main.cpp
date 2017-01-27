#include "im3d_example.h"

int main(int, char**)
{
	Im3d::Example example;
	if (!example.init(-1, -1, "Im3d Example")) {
		return 1;
	}

	while (example.update()) {
		Im3d::RandSeed(0);

		Im3d::Context& ctx = Im3d::GetContext();
		Im3d::AppData& ad  = Im3d::GetAppData();

		static const int   kGridSize = 20;
		static const float kGridHalf = (float)kGridSize * 0.5f;
		Im3d::PushDrawState();
			static bool s_showGrid = false;
			ImGui::Checkbox("Show Grid", &s_showGrid);
			if (s_showGrid) {
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
			}
			if (ImGui::TreeNode("Intersection")) {
				Im3d::PushDrawState();
					Im3d::Ray ray(ad.m_cursorRayOrigin, ad.m_cursorRayDirection);
					float tr;
					Im3d::Plane plane(Im3d::Vec3(0.0f, 1.0f, 0.0f), 0.0f);
					if (Im3d::Intersect(ray, plane, tr)) {
						Im3d::BeginPoints();
							Im3d::Vertex(ray.m_origin + ray.m_direction * tr, 8.0f, Im3d::Color_Magenta);
						Im3d::End();
					}
				Im3d::PopDrawState();

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Gizmos")) {
				int gizmoMode = (int)Im3d::GetContext().getGizmoMode();
				ImGui::RadioButton("Translate (Ctrl+T)", &gizmoMode, Im3d::GizmoMode_Translation); 
				ImGui::SameLine();
				ImGui::RadioButton("Rotate (Ctrl+R)", &gizmoMode, Im3d::GizmoMode_Rotation);
				ImGui::SameLine();
				ImGui::RadioButton("Scale (Ctrl+S)", &gizmoMode, Im3d::GizmoMode_Scale);
				Im3d::GetContext().setGizmoMode((Im3d::GizmoMode)gizmoMode);
				
				ImGui::SliderFloat("Gizmo Size", &ctx.m_gizmoHeightPixels, 0.0f, 256.0f);
				ImGui::SliderFloat("Gizmo Thickness", &ctx.m_gizmoSizePixels, 0.0f, 32.0f);
				ImGui::Text("Hot ID:    0x%x", ctx.m_idHot);
				ImGui::Text("Active ID: 0x%x", ctx.m_idActive);
				ImGui::Text("Hot Depth: %.3f", ctx.m_hotDepth == FLT_MAX ? -1.0f : ctx.m_hotDepth);
				static Im3d::Mat4 m(1.0f);
				Im3d::PushDrawState();
					Im3d::PushMatrix();
						Im3d::SetAlpha(0.5f);
						Im3d::SetSize(2.0f);
						Im3d::SetMatrix(m);
						Im3d::DrawXyzAxes();
						Im3d::SetColor(1.0f, 0.1f, 0.4f);
						Im3d::SetSize(4.0f);
						Im3d::DrawAlignedBox(Im3d::Vec3(-0.5f), Im3d::Vec3(0.5f));
					Im3d::PopMatrix();
				Im3d::PopDrawState();
				Im3d::Gizmo("GizmoTest", m); // transform after drawing the object to avoid 1 frame lag
				
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Sorting")) {
				static bool s_enableSorting = true;
				static int  s_primCount = 1000;
				ImGui::Checkbox("Enable sorting", &s_enableSorting);
				ImGui::SliderInt("Prim Count", &s_primCount, 2, 10000);

				Im3d::PushDrawState();
					Im3d::EnableSorting(s_enableSorting);
					Im3d::SetAlpha(1.0f);
					for (int i = 0; i < s_primCount / 2; ++i) {
						Im3d::PushMatrix();
							Im3d::Mat4 wm(1.0f);
							wm.setRotationScale(Im3d::Rotation(Im3d::Normalize(Im3d::RandVec3(-1.0f, 1.0f)), Im3d::RandFloat(0.0f, 6.0f)));
							wm.setTranslation(Im3d::RandVec3(-10.0f, 10.0f));
							Im3d::MulMatrix(wm);
							Im3d::BeginTriangles();
								Im3d::Vertex(-1.0f,  0.0f, -1.0f, Im3d::Color_Red);
								Im3d::Vertex( 0.0f,  2.0f, -1.0f, Im3d::Color_Green);
								Im3d::Vertex( 1.0f,  0.0f, -1.0f, Im3d::Color_Blue);
							Im3d::End();
						Im3d::PopMatrix();
					}

					Im3d::SetAlpha(1.0f);
					Im3d::SetSize(2.5f);
					for (int i = 0; i < s_primCount / 2; ++i) {
						Im3d::PushMatrix();
							Im3d::Mat4 wm(1.0f);
							wm.setRotationScale(Im3d::Rotation(Im3d::Normalize(Im3d::RandVec3(-1.0f, 1.0f)), Im3d::RandFloat(0.0f, 6.0f)));
							wm.setTranslation(Im3d::RandVec3(-10.0f, 10.0f));
							Im3d::MulMatrix(wm);
							Im3d::BeginLineLoop();
								Im3d::Vertex(-1.0f,  0.0f, -1.0f, Im3d::Color_Magenta);
								Im3d::Vertex( 0.0f,  2.0f, -1.0f, Im3d::Color_Yellow);
								Im3d::Vertex( 1.0f,  0.0f, -1.0f, Im3d::Color_Cyan);
							Im3d::End();
						Im3d::PopMatrix();
					}
				Im3d::PopDrawState();

				ImGui::TreePop();
			}

		Im3d::PopDrawState();


		example.draw();
	}
	example.shutdown();
	
	return 0;
}
