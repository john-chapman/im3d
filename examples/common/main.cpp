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

		ImGui::Begin("Im3d Demo");
		Im3d::PushDrawState();
		
		ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_Once);
		if (ImGui::TreeNode("About")) {
			ImGui::Text("Welcome to the Im3d demo!");
			ImGui::Spacing();
			ImGui::Text("WASD = camera position + QE for down/up");
			ImGui::Text("L Shift = go faster");
			ImGui::Text("R Mouse + drag = camera orientation");
			ImGui::Spacing();

			ImGui::TreePop();
		}
		ImGui::Spacing();

		ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_Once);
		if (ImGui::TreeNode("Grid")) {
			static int gridSize = 20;
			ImGui::SliderInt("Grid Size", &gridSize, 1, 50);
			const float gridHalf = (float)gridSize * 0.5f;
			Im3d::SetAlpha(1.0f);
			Im3d::SetSize(1.0f);
			Im3d::BeginLines();
				for (int x = 0; x <= gridSize; ++x) {
					Im3d::Vertex(-gridHalf, 0.0f, (float)x - gridHalf, Im3d::Color(0.0f, 0.0f, 0.0f));
					Im3d::Vertex( gridHalf, 0.0f, (float)x - gridHalf, Im3d::Color(1.0f, 0.0f, 0.0f));
				}
				for (int z = 0; z <= gridSize; ++z) {
					Im3d::Vertex((float)z - gridHalf, 0.0f, -gridHalf,  Im3d::Color(0.0f, 0.0f, 0.0f));
					Im3d::Vertex((float)z - gridHalf, 0.0f,  gridHalf,  Im3d::Color(0.0f, 0.0f, 1.0f));
				}
			Im3d::End();

			ImGui::TreePop();
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

		if (ImGui::TreeNode("Shapes")) {
			Im3d::PushDrawState();

			static Im3d::Mat4 m(1.0f);
			Im3d::Gizmo("ShapeGizmo", m);
			Im3d::PushMatrix(m);
				Im3d::SetColor(Im3d::Color_Magenta);
				Im3d::SetSize(4.0f);
				//Im3d::DrawQuad(Im3d::Vec3(0.0f), Im3d::Vec3(0.0f, 0.0f, 1.0f), Im3d::Vec2(1.0f));
				Im3d::DrawCapsule(Im3d::Vec3(-1.0f, -1.0f, 0.0f), Im3d::Vec3(1.0f, 1.0f, 0.0f), 1.0f);
			Im3d::PopMatrix();
			/*static Im3d::Vec3 position;
			Im3d::GizmoTranslation("LodPosition", &position.x);
			static float radius = 1.0f;
			ImGui::SliderFloat("Radius", &radius, 0.0f, 20.0f); 
			static int lodMinMax[2] = { 16, 256 };
			ImGui::SliderInt2("LOD Range", lodMinMax, 3, 256);
			lodMinMax[0] = Im3d::Min(lodMinMax[0], lodMinMax[1]);
			lodMinMax[1] = Im3d::Max(lodMinMax[0], lodMinMax[1]);
			int lod = Im3d::GetContext().estimateLevelOfDetail(position, radius, lodMinMax[0], lodMinMax[1]);
			ImGui::Text("LOD %d", lod);

			Im3d::PushSize(4.0f);
			Im3d::PushColor(Im3d::Color(1.0f, 0.4f, 0.0f));
			Im3d::DrawCylinder(position + Im3d::Vec3(-radius, 0.0f, 0.0f), position + Im3d::Vec3(radius, 0.0f, 0.0f), radius, lod);
			Im3d::PopColor();
			Im3d::PopSize();*/
			
			Im3d::PopDrawState();
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Gizmo")) {
			ImGui::Text("Hot ID: 0x%x  Active ID: 0x%x  Hot Depth: %.3f", ctx.m_hotId, ctx.m_activeId, ctx.m_hotDepth);
			
			ImGui::Spacing();
			ImGui::Checkbox("Local", &Im3d::GetContext().m_gizmoLocal);
			int gizmoMode = (int)Im3d::GetContext().m_gizmoMode;
			ImGui::RadioButton("Translate (Ctrl+T)", &gizmoMode, Im3d::GizmoMode_Translation); 
			ImGui::SameLine();
			ImGui::RadioButton("Rotate (Ctrl+R)", &gizmoMode, Im3d::GizmoMode_Rotation);
			ImGui::SameLine();
			ImGui::RadioButton("Scale (Ctrl+S)", &gizmoMode, Im3d::GizmoMode_Scale);
			Im3d::GetContext().m_gizmoMode = (Im3d::GizmoMode)gizmoMode;
			
			ImGui::Spacing();
			ImGui::SliderFloat("Gizmo Size", &ctx.m_gizmoHeightPixels, 0.0f, 256.0f);
			ImGui::SliderFloat("Gizmo Thickness", &ctx.m_gizmoSizePixels, 0.0f, 32.0f);
		
			static bool first = true;
			static Im3d::Mat4 transforms[8];
			if (first) {
				transforms[0] = Im3d::Mat4(1.0f);
				for (int i = 1; i < 8; ++i) {
					transforms[i] = Im3d::Mat4(
						Im3d::RandVec3(-10.0f, 10.0f),
						Im3d::RandRotation(),
						Im3d::Vec3(1.0f)
						);
				}
				first = false;
			}

			for (int i = 0; i < 8; ++i) {
			 // drawing the object first avoids 1 frame lag
				Im3d::DrawTeapot(transforms[i], example.m_camViewProj);
				Im3d::PushId((Im3d::Id)(i + 1));
				Im3d::Gizmo("GizmoTest", transforms[i], ctx.m_gizmoLocal);
				Im3d::PopId();
			}
			
			ImGui::TreePop();
		}
		
		if (ImGui::TreeNode("Basic Perf")) {
			static bool s_enableSorting = false;
			static bool s_useMatrix = false;
			static int  s_primCount = 50000;
			ImGui::Checkbox("Enable sorting", &s_enableSorting);
			ImGui::Checkbox("Use matrix stack", &s_useMatrix);
			ImGui::SliderInt("Prim Count", &s_primCount, 2, 50000);
			
			Im3d::PushEnableSorting(s_enableSorting);
			Im3d::BeginPoints();
			if (s_useMatrix) {
				Im3d::PushMatrix();
				for (int i = 0; i < s_primCount; ++i) {
					Im3d::Mat4 wm(1.0f);
					wm.setTranslation(Im3d::RandVec3(-10.0f, 10.0f));
					Im3d::SetMatrix(wm);
					Im3d::Vertex(Im3d::Vec3(0.0f), Im3d::RandFloat(2.0f, 16.0f), Im3d::RandColor(0.0f, 1.0f));
				}
				Im3d::PopMatrix();
			} else {
				for (int i = 0; i < s_primCount; ++i) {
					Im3d::Vec3 t = Im3d::RandVec3(-10.0f, 10.0f);
					Im3d::Vertex(t, Im3d::RandFloat(2.0f, 16.0f), Im3d::RandColor(0.0f, 1.0f));
				}
			}
			Im3d::End();
			Im3d::PopEnableSorting();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Sorting")) {
			static bool s_enableSorting = true;
			static int  s_primCount = 1000;
			ImGui::Checkbox("Enable sorting", &s_enableSorting);
			ImGui::SliderInt("Prim Count", &s_primCount, 2, 10000);

			Im3d::PushDrawState();
				Im3d::EnableSorting(s_enableSorting);
				Im3d::SetAlpha(0.9f);
				for (int i = 0; i < s_primCount / 3; ++i) {
					Im3d::PushMatrix();
						Im3d::Mat4 wm(1.0f);
						wm.setRotation(Im3d::Rotation(Im3d::Normalize(Im3d::RandVec3(-1.0f, 1.0f)), Im3d::RandFloat(0.0f, 6.0f)));
						wm.setTranslation(Im3d::RandVec3(-10.0f, 10.0f));
						Im3d::MulMatrix(wm);
						Im3d::BeginTriangles();
							Im3d::Vertex(-1.0f,  0.0f, -1.0f, Im3d::Color_Red);
							Im3d::Vertex( 0.0f,  2.0f, -1.0f, Im3d::Color_Green);
							Im3d::Vertex( 1.0f,  0.0f, -1.0f, Im3d::Color_Blue);
						Im3d::End();
					Im3d::PopMatrix();
				}

				Im3d::SetAlpha(0.9f);
				Im3d::SetSize(2.5f);
				for (int i = 0; i < s_primCount / 3 / 3; ++i) {
					Im3d::PushMatrix();
						Im3d::Mat4 wm(1.0f);
						wm.setRotation(Im3d::Rotation(Im3d::Normalize(Im3d::RandVec3(-1.0f, 1.0f)), Im3d::RandFloat(0.0f, 6.0f)));
						wm.setTranslation(Im3d::RandVec3(-10.0f, 10.0f));
						Im3d::MulMatrix(wm);
						Im3d::BeginLineLoop();
							Im3d::Vertex(-1.0f,  0.0f, -1.0f, Im3d::Color_Magenta);
							Im3d::Vertex( 0.0f,  2.0f, -1.0f, Im3d::Color_Yellow);
							Im3d::Vertex( 1.0f,  0.0f, -1.0f, Im3d::Color_Cyan);
						Im3d::End();
					Im3d::PopMatrix();
				}

				Im3d::SetAlpha(0.9f);
				Im3d::SetSize(16.0f);
				for (int i = 0; i < s_primCount / 3; ++i) {
					Im3d::PushMatrix();
						Im3d::Mat4 wm(1.0f);
						wm.setTranslation(Im3d::RandVec3(-10.0f, 10.0f));
						Im3d::MulMatrix(wm);
						Im3d::BeginPoints();
							Im3d::Vertex(-1.0f,  0.0f, -1.0f, Im3d::RandColor(0.0f, 1.0f));
						Im3d::End();
					Im3d::PopMatrix();
				}
			Im3d::PopDrawState();

			ImGui::TreePop();
		}

		Im3d::PopDrawState();
		ImGui::End();

		example.draw();
	}
	example.shutdown();
	
	return 0;
}
