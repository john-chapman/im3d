#include "im3d_example.h"

int main(int, char**)
{
	Im3d::Example example;
	if (!example.init(-1, -1, "Im3d Example")) {
		return 1;
	}

	while (example.update()) {
		Im3d::RandSeed(0);

		static const int   kGridSize = 20;
		static const float kGridHalf = (float)kGridSize * 0.5f;
		Im3d::PushDrawState();
			static float sz = 1.0f;
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
		
			//Im3d::BeginTriangles();
			//	Im3d::Vertex(-1.0f,  0.0f, -1.0f, Im3d::Color_Red);
			//	Im3d::Vertex( 0.0f,  2.0f, -1.0f, Im3d::Color_Green);
			//	Im3d::Vertex( 1.0f,  0.0f, -1.0f, Im3d::Color_Blue);
			//Im3d::End();
			//Im3d::SetSize(2.0f);
			//Im3d::BeginLineLoop();
			//	Im3d::Vertex(-1.0f,  0.0f, -1.0f, Im3d::Color_Magenta);
			//	Im3d::Vertex( 0.0f,  2.0f, -1.0f, Im3d::Color_Yellow);
			//	Im3d::Vertex( 1.0f,  0.0f, -1.0f, Im3d::Color_Cyan);
			//Im3d::End();

			if (ImGui::TreeNode("Gizmos")) {
				ImGui::Text("Hot ID:    0x%x", Im3d::GetContext().m_idHot);
				ImGui::Text("Active ID: 0x%x", Im3d::GetContext().m_idActive);
				ImGui::Text("Hot Depth: %.3f", Im3d::GetContext().m_hotDepth);
				static Im3d::Vec3 pos0(0.0f, 0.0f, 0.0f);
				Im3d::GizmoPosition("GizmoPos0", &pos0);

				static Im3d::Vec3 pos1(0.0f, 0.0f, -1.0f);
				Im3d::GizmoPosition("GizmoPos1", &pos1);

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Sorting")) {
				static bool s_enableSorting = true;
				static int  s_primCount = 1000;
				ImGui::Checkbox("Enable sorting", &s_enableSorting);
				ImGui::SliderInt("Prim Count", &s_primCount, 2, 10000);

				Im3d::PushDrawState();
					Im3d::EnableSorting(s_enableSorting);
					Im3d::SetAlpha(0.75f);
					for (int i = 0; i < s_primCount / 2; ++i) {
						Im3d::PushMatrix();
							Im3d::Mat4 wm(1.0f);
							wm = Im3d::Rotate(wm, Im3d::Normalize(Im3d::RandVec3(-1.0f, 1.0f)), Im3d::RandFloat(0.0f, 6.0f));
							wm = Im3d::Translate(wm, Im3d::RandVec3(-10.0f, 10.0f));
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
							wm = Im3d::Rotate(wm, Im3d::Normalize(Im3d::RandVec3(-1.0f, 1.0f)), Im3d::RandFloat(0.0f, 6.0f));
							wm = Im3d::Translate(wm, Im3d::RandVec3(-10.0f, 10.0f));
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
