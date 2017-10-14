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

		ImGui::Begin("Im3d Demo", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_Once);
		if (ImGui::TreeNode("About")) {
			ImGui::Text("Welcome to the Im3d demo!");
			ImGui::Spacing();
			ImGui::Text("WASD   = forward/left/backward/right");
			ImGui::Text("QE     = down/up");
			ImGui::Text("RMouse = camera orientation");
			ImGui::Text("LShift = move faster");
			ImGui::Spacing();

			ImGui::TreePop();
		}
		ImGui::Spacing();

		ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_Once);
		if (ImGui::TreeNode("Unified Gizmo")) {
		 // Unified gizmo operates directly on a 4x4 matrix using the context-global gizmo modes.
			static Im3d::Mat4 transform(1.0f);

		 // Context-global gizmo modes are set via actions in the AppData::m_keyDown but could also be modified via a GUI as follows:
			int gizmoMode = (int)Im3d::GetContext().m_gizmoMode;
			ImGui::Checkbox("Local (Ctrl+L)", &Im3d::GetContext().m_gizmoLocal);
			ImGui::SameLine();
			ImGui::RadioButton("Translate (Ctrl+T)", &gizmoMode, Im3d::GizmoMode_Translation); 
			ImGui::SameLine();
			ImGui::RadioButton("Rotate (Ctrl+R)", &gizmoMode, Im3d::GizmoMode_Rotation);
			ImGui::SameLine();
			ImGui::RadioButton("Scale (Ctrl+S)", &gizmoMode, Im3d::GizmoMode_Scale);
			Im3d::GetContext().m_gizmoMode = (Im3d::GizmoMode)gizmoMode;
	
		 // The ID passed to Gizmo() should be unique during a frame - to create gizmos in a loop use PushId()/PopId().
			if (Im3d::Gizmo("GizmoUnified", transform)) {
			 // if Gizmo() returns true, the transform was modified
				switch (Im3d::GetContext().m_gizmoMode) {
					case Im3d::GizmoMode_Translation: {
						Im3d::Vec3 pos = transform.getTranslation();
						ImGui::Text("Position: %.3f, %.3f, %.3f", pos.x, pos.y, pos.z);
						break;
					}
					case Im3d::GizmoMode_Rotation: {
						Im3d::Vec3 euler = Im3d::ToEulerXYZ(transform.getRotation());
						ImGui::Text("Rotation: %.3f, %.3f, %.3f", Im3d::Degrees(euler.x), Im3d::Degrees(euler.y), Im3d::Degrees(euler.z));
						break;
					}
					case Im3d::GizmoMode_Scale: {
						Im3d::Vec3 scale = transform.getScale();
						ImGui::Text("Scale: %.3f, %.3f, %.3f", scale.x, scale.y, scale.z);
						break;
					}
					default: break;
				};
				
			}

		 // Using the transform for drawing *after* the call to Gizmo() causes a 1 frame lag between the gizmo position and the output
		 // matrix - this can only be avoided if it's possible to issue the draw call *before* calling Gizmo().
			Im3d::DrawTeapot(transform, example.m_camViewProj);

			ImGui::TreePop();
		}


		if (ImGui::TreeNode("Separate Gizmos")) {
		 // Translation/rotation/scale can be modified separately - useful in cases where only certain transformations are valid.
			static Im3d::Vec3 translation(0.0f);
			static Im3d::Mat3 rotation(1.0f);
			static Im3d::Vec3 scale(1.0f);

		 // The separate Gizmo*() functions require the transformation to be pushed on the matrix stack to correctly handle local gizmos.
			Im3d::PushMatrix(Im3d::Mat4(translation, rotation, scale));

			int gizmoMode = (int)Im3d::GetContext().m_gizmoMode;
			ImGui::Checkbox("Local (Ctrl+L)", &Im3d::GetContext().m_gizmoLocal);
			ImGui::SameLine();
			ImGui::RadioButton("Translate (Ctrl+T)", &gizmoMode, Im3d::GizmoMode_Translation); 
			ImGui::SameLine();
			ImGui::RadioButton("Rotate (Ctrl+R)", &gizmoMode, Im3d::GizmoMode_Rotation);
			ImGui::SameLine();
			ImGui::RadioButton("Scale (Ctrl+S)", &gizmoMode, Im3d::GizmoMode_Scale);
			Im3d::GetContext().m_gizmoMode = (Im3d::GizmoMode)gizmoMode;

			switch (Im3d::GetContext().m_gizmoMode) {
				case Im3d::GizmoMode_Translation:
					if (Im3d::GizmoTranslation("GizmoTranslation", translation, Im3d::GetContext().m_gizmoLocal)) {
						ImGui::Text("Position: %.3f, %.3f, %.3f", translation.x, translation.y, translation.z);
					}
					break;
				case Im3d::GizmoMode_Rotation: {
					if (Im3d::GizmoRotation("GizmoRotation", rotation, Im3d::GetContext().m_gizmoLocal)) {
						Im3d::Vec3 euler = Im3d::ToEulerXYZ(rotation);
						ImGui::Text("Rotation: %.3f, %.3f, %.3f", Im3d::Degrees(euler.x), Im3d::Degrees(euler.y), Im3d::Degrees(euler.z));
					}
					break;
				}
				case Im3d::GizmoMode_Scale: {
					if (Im3d::GizmoScale("GizmoScale", scale)) {
						ImGui::Text("Scale: %.3f, %.3f, %.3f", scale.x, scale.y, scale.z);
					}
					break;
				}
				default: break;
			};

			Im3d::DrawTeapot(Im3d::Mat4(translation, rotation, scale), example.m_camViewProj);

			Im3d::PopMatrix();

			ImGui::TreePop();
		}

		
		if (ImGui::TreeNode("Hierarchical Gizmos")) {
		 // It is often useful to modify a single node in a transformation hierarchy directly, which can be done as follows.
		 // Note that scaling the parent is probably undesirable in these cases.
			static Im3d::Mat4 parent(1.0f);
			static Im3d::Mat4 child(Im3d::Vec3(0.0f, 1.0f, 0.0f), Im3d::Mat3(1.0f), Im3d::Vec3(0.5f));

			Im3d::Gizmo("GizmoParent", parent); // modify parent directly
			
			Im3d::Mat4 parentChild = parent * child; // modify the final world space transform
			if (Im3d::Gizmo("GizmoChild", parentChild)) {
				child = Im3d::Inverse(parent) * parentChild; // extract the child transform if modified
			}
			
			Im3d::DrawTeapot(parent, example.m_camViewProj);
			Im3d::DrawTeapot(parent * child, example.m_camViewProj);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Gizmo Appearance")) {
		 // The size/radius of gizmos can be modified globally.
			static float alpha  = 1.0f;
			static float size   = Im3d::GetContext().m_gizmoSizePixels;
			static float height = Im3d::GetContext().m_gizmoHeightPixels; 

			ImGui::SliderFloat("Alpha", &alpha, 0.0f, 1.0f);
			ImGui::SliderFloat("Height/Radius", &height, 0.0f, 128.0f);
			ImGui::SliderFloat("Thickness", &size, 0.0f, 16.0f);

			Im3d::PushAlpha(alpha);
			float storedSize = Im3d::GetContext().m_gizmoSizePixels;			
			Im3d::GetContext().m_gizmoSizePixels = size;
			float storedHeight = Im3d::GetContext().m_gizmoHeightPixels;
			Im3d::GetContext().m_gizmoHeightPixels = height;

			static Im3d::Mat4 transform(1.0f);
			Im3d::Gizmo("GizmoAppearance", transform);
			Im3d::DrawTeapot(transform, example.m_camViewProj);

			Im3d::GetContext().m_gizmoHeightPixels = storedHeight;
			Im3d::GetContext().m_gizmoSizePixels = storedSize;
			Im3d::PopAlpha();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Cursor Ray Intersection")) {
		 // Context exposes the 'hot depth' along the cursor ray which intersects with the current hot gizmo - this is useful
		 // when drawing the cursor ray.
			float depth = FLT_MAX;
			depth = Im3d::Min(depth, Im3d::GetContext().m_hotDepth);
			float size = Im3d::Clamp(32.0f / depth, 4.0f, 32.0f);
			
			if (depth != FLT_MAX) {
				ImGui::Text("Depth: %f", depth);
				Im3d::PushEnableSorting(true);
				Im3d::BeginPoints();
					Im3d::Vertex(ad.m_cursorRayOrigin + ad.m_cursorRayDirection * depth * 0.99f, size, Im3d::Color_Magenta);
				Im3d::End();
				Im3d::PopEnableSorting();
			} else {
				ImGui::Text("Depth: FLT_MAX");
			}
		
			ImGui::TreePop();
		}

		
		if (ImGui::TreeNode("High Order Shapes")) {
		 // Im3d provides functions to easily draw high order shapes - these don't strictly require a matrix to be pushed on
		 // the stack (although this is supported, as below).
			static Im3d::Mat4 transform(1.0f);
			Im3d::Gizmo("ShapeGizmo", transform);

			enum Shape {
				Shape_Quad,
				Shape_QuadFilled,
				Shape_Circle,
				Shape_CircleFilled,
				Shape_Sphere,
				Shape_AlignedBox,
				Shape_Cylinder,
				Shape_Capsule,
				Shape_Prism,
				Shape_Arrow
			};
			static const char* shapeList = 
				"Quad\0"
				"Quad Filled\0"
				"Circle\0"
				"Circle Filled\0"
				"Sphere\0"
				"AlignedBox\0"
				"Cylinder\0"
				"Capsule\0"
				"Prism\0"
				"Arrow\0"
				;
			static int currentShape = Shape_Capsule;
			ImGui::Combo("Shape", &currentShape, shapeList);
			static Im3d::Vec4 color = Im3d::Vec4(1.0f, 0.0f, 0.6f, 1.0f);
			ImGui::ColorEdit4("Color", color);
			
			static float thickness = 4.0f;
			ImGui::SliderFloat("Thickness", &thickness, 0.0f, 16.0f);
			
			Im3d::PushMatrix(transform);
			Im3d::PushDrawState();
			Im3d::SetSize(thickness);
			Im3d::SetColor(Im3d::Color(color.x, color.y, color.z, color.w));

			switch ((Shape)currentShape) {
				case Shape_Quad: 
				case Shape_QuadFilled: {
					static Im3d::Vec2 quadSize(1.0f);
					ImGui::SliderFloat2("Size", quadSize, 0.0f, 10.0f);
					if (currentShape == Shape_Quad) {
						Im3d::DrawQuad(Im3d::Vec3(0.0f), Im3d::Vec3(0.0f, 0.0f, 1.0f), quadSize);
					} else {
						Im3d::DrawQuadFilled(Im3d::Vec3(0.0f), Im3d::Vec3(0.0f, 0.0f, 1.0f), quadSize);
					}
					break;
				}
				case Shape_Circle: 
				case Shape_CircleFilled: {
					static float circleRadius = 1.0f;
					ImGui::SliderFloat("Radius", &circleRadius, 0.0f, 10.0f);
					if (currentShape == Shape_Circle) {
						Im3d::DrawCircle(Im3d::Vec3(0.0f), Im3d::Vec3(0.0f, 0.0f, 1.0f), circleRadius);
					} else if (currentShape = Shape_CircleFilled) {
						Im3d::DrawCircleFilled(Im3d::Vec3(0.0f), Im3d::Vec3(0.0f, 0.0f, 1.0f), circleRadius);
					}
					break;
				}
				case Shape_Sphere: {
					static float sphereRadius = 1.0f;
					ImGui::SliderFloat("Radius", &sphereRadius, 0.0f, 10.0f);
					Im3d::DrawSphere(Im3d::Vec3(0.0f), sphereRadius);
					break;
				}
				case Shape_AlignedBox: {
					static Im3d::Vec3 boxSize(1.0f);
					ImGui::SliderFloat3("Size", boxSize, 0.0f, 10.0f);
					Im3d::DrawAlignedBox(-boxSize, boxSize);
					break;
				}
				case Shape_Cylinder: {
					static float cylinderRadius = 1.0f;
					static float cylinderLength = 1.0f;
					ImGui::SliderFloat("Radius", &cylinderRadius, 0.0f, 10.0f);
					ImGui::SliderFloat("Length", &cylinderLength, 0.0f, 10.0f);
					Im3d::DrawCylinder(Im3d::Vec3(0.0f, -cylinderLength, 0.0f), Im3d::Vec3(0.0f, cylinderLength, 0.0f), cylinderRadius);
					break;
				}
				case Shape_Capsule: {
					static float capsuleRadius = 0.5f;
					static float capsuleLength = 1.0f;
					ImGui::SliderFloat("Radius", &capsuleRadius, 0.0f, 10.0f);
					ImGui::SliderFloat("Length", &capsuleLength, 0.0f, 10.0f);
					Im3d::DrawCapsule(Im3d::Vec3(0.0f, -capsuleLength, 0.0f), Im3d::Vec3(0.0f, capsuleLength, 0.0f), capsuleRadius);
					break;
				}
				case Shape_Prism: {
					static float prismRadius = 1.0f;
					static float prismLength = 1.0f;
					static int   prismSides  = 3;
					ImGui::SliderFloat("Radius", &prismRadius, 0.0f, 10.0f);
					ImGui::SliderFloat("Length", &prismLength, 0.0f, 10.0f);
					ImGui::SliderInt("Sides", &prismSides, 3, 16);
					Im3d::DrawPrism(Im3d::Vec3(0.0f, -prismLength, 0.0f), Im3d::Vec3(0.0f, prismLength, 0.0f), prismRadius, prismSides);
					break;
				}
				case Shape_Arrow: {
					static float arrowLength   = 1.0f;
					static float headLength    = -1.0f;
					static float headThickness = -1.0f;
					ImGui::SliderFloat("Length",          &arrowLength,   0.0f, 10.0f);
					ImGui::SliderFloat("Head Length",     &headLength,    0.0f, 1.0f);
					ImGui::SliderFloat("Head Thickness",  &headThickness, 0.0f, 1.0f);
					Im3d::DrawArrow(Im3d::Vec3(0.0f), Im3d::Vec3(0.0f, arrowLength, 0.0f), headLength, headThickness);
					break;
				}
				default:
					break;
			};

			Im3d::PopDrawState();
			Im3d::PopMatrix();

			ImGui::TreePop();
		}

		
		if (ImGui::TreeNode("Basic Perf")) {
		 // Simple perf test: draw a large number of points, enable/disable sorting and the use of the matrix stack.
			static bool enableSorting = false;
			static bool useMatrix = false; // if the matrix stack size == 1 Im3d assumes it's the identity matrix and skips the matrix mul as an optimisation
			static int  primCount = 50000;
			ImGui::Checkbox("Enable sorting", &enableSorting);
			ImGui::Checkbox("Use matrix stack", &useMatrix);
			ImGui::SliderInt("Prim Count", &primCount, 2, 100000);
			
			Im3d::PushEnableSorting(enableSorting);
			Im3d::BeginPoints();
			if (useMatrix) {
				Im3d::PushMatrix();
				for (int i = 0; i < primCount; ++i) {
					Im3d::Mat4 wm(1.0f);
					wm.setTranslation(Im3d::RandVec3(-10.0f, 10.0f));
					Im3d::SetMatrix(wm);
					Im3d::Vertex(Im3d::Vec3(0.0f), Im3d::RandFloat(2.0f, 16.0f), Im3d::RandColor(0.0f, 1.0f));
				}
				Im3d::PopMatrix();
			} else {
				for (int i = 0; i < primCount; ++i) {
					Im3d::Vec3 t = Im3d::RandVec3(-10.0f, 10.0f);
					Im3d::Vertex(t, Im3d::RandFloat(2.0f, 16.0f), Im3d::RandColor(0.0f, 1.0f));
				}
			}
			Im3d::End();
			Im3d::PopEnableSorting();

			ImGui::TreePop();
		}


		if (ImGui::TreeNode("Sorting")) {
		 // If sorting is enabled, primitives are sorted back-to-front for rendering. Lines/triangles use the primitive midpoint, so very long
		 // lines or large triangles may not sort correctly.
			static bool enableSorting = true;
			static int  primCount = 1000;
			ImGui::Checkbox("Enable sorting", &enableSorting);
			ImGui::SliderInt("Prim Count", &primCount, 2, 10000);

			Im3d::PushDrawState();
				Im3d::EnableSorting(enableSorting);
				Im3d::SetAlpha(0.9f);
				for (int i = 0; i < primCount / 3; ++i) {
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
				for (int i = 0; i < primCount / 3 / 3; ++i) {
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
				for (int i = 0; i < primCount / 3; ++i) {
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
		
		if (ImGui::TreeNode("Layers")) {
		 // Layers allow primitives to be grouped by the application. Each layer results in a separate call to the draw callback, which gives the 
		 // application opportunity to modify the rendering on a per-layer basis (e.g. to enable depth testing). Layers can also be used to 
		 // achieve some coarse-grained sorting, as below:

		 // Layers are drawn in the order which the application declares them.
			Im3d::PushLayerId(Im3d::MakeId("DrawFirst")); Im3d::PopLayerId();
			Im3d::PushLayerId(Im3d::MakeId("DrawSecond")); Im3d::PopLayerId();

			Im3d::PushLayerId(Im3d::MakeId("DrawSecond"));
				Im3d::BeginTriangles();
					Im3d::Vertex(-0.4f, 0.0f, 0.0f, 16.0f, Im3d::Color_Red);
					Im3d::Vertex( 0.1f, 1.0f, 0.0f, 16.0f, Im3d::Color_Red);
					Im3d::Vertex( 0.6f, 0.0f, 0.0f, 16.0f, Im3d::Color_Red);
				Im3d::End();
			Im3d::PopLayerId();
			Im3d::PushLayerId(Im3d::MakeId("DrawFirst"));
				Im3d::BeginTriangles();
					Im3d::Vertex(-0.6f, 0.0f, 0.0f, 16.0f, Im3d::Color_Magenta);
					Im3d::Vertex(-0.1f, 1.0f, 0.0f, 16.0f, Im3d::Color_Magenta);
					Im3d::Vertex( 0.4f, 0.0f, 0.0f, 16.0f, Im3d::Color_Magenta);
				Im3d::End();
			Im3d::PopLayerId();

			ImGui::TreePop();
		}


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

		if (ImGui::TreeNode("Camera")) {
			ImGui::Checkbox("Ortho", &example.m_camOrtho);

			ImGui::TreePop();
		}

		ImGui::End();

		example.draw();
	}
	example.shutdown();
	
	return 0;
}
