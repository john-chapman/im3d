/*	Multi context example
	This example shows how to use multiple Im3d contexts to draw from multiple threads. There are
	several steps to make this work:

	1) #define IM3D_THREAD_LOCAL_CONTEXT_PTR 1 - either modify im3d_config.h or define via the build
	system. This is required to declare Im3d's internal context ptr as thread_local which allows the
	application to set a different context per thread.

	2) Declare some number of Im3d::Context instances (e.g. 1 per thread) - the internal context ptr
	is thread local but points to the default context.

*/
#include "im3d_example.h"

#include <thread>

static const int kThreadCount = 6;

static Im3d::Context g_ContextPool[kThreadCount];

static void MainThreadDraw();

int main(int, char**)
{
	Im3d::Example example;
	if (!example.init(-1, -1, "Im3d Example")) {
		return 1;
	}


	while (example.update()) { // calls Im3d_Update() (see im3d_opengl33.cpp)
	// At this point we have updated the default context and filled its AppData struct. 


		MainThreadDraw();

	// Prior to calling Im3d we need to merge the per-thread contexts into the main thread context.

		example.draw(); // calls Im3d_Draw() (see im3d_opengl33.cpp).
	}
	example.shutdown();
	
	return 0;
}

void MainThreadDraw()
{
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

	ImGui::End();
}