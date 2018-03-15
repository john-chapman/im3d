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

static const int     kThreadCountMax = 6;
static bool          g_EnableSorting = false;
static int           g_ThreadCount   = kThreadCountMax;
static Im3d::Context g_ThreadContexts[kThreadCountMax];
static Im3d::Color   g_ThreadColors[kThreadCountMax] = 
{
	Im3d::Color_Red,
	Im3d::Color_Green,
	Im3d::Color_Blue,
	Im3d::Color_Magenta,
	Im3d::Color_Yellow,
	Im3d::Color_Cyan
};

static void ThreadDraw(int _threadIndex);
static void MainThreadDraw();

int main(int, char**)
{
	Im3d::Example example;
	if (!example.init(-1, -1, "Im3d Example")) {
		return 1;
	}
//TODO
//- add a method on the context to get a debug string (with the ID etc)
//- update the FAQ with a note about thread safety.
//- Wiki about the config options.

	while (example.update()) { // calls Im3d_Update() (see im3d_opengl33.cpp)
	// At this point we have updated the default context and filled its AppData struct. 

	// Each separate context could potentially use different AppData (e.g. different camera settings). Here 
	// we just copy the default for simplicity.
		for (auto& ctx : g_ThreadContexts) {
			ctx.getAppData() = Im3d::GetAppData();

			ctx.reset(); // equivalent to calling Im3d::NewFrame() inside the thread
		}

	// The frame can now proceed, threads can safely make Im3d calls.
		std::thread threads[kThreadCountMax];
		for (int i = 0; i < g_ThreadCount; ++i) {
			threads[i] = std::thread(&ThreadDraw, i);
		}		
		MainThreadDraw();

	// Towards the end of the frame we need a global sync point so that we can safely merge vertex data from the
	// per-thread contexts.
		for (auto& thread : threads) {
			if (thread.joinable()) {
				thread.join();
			}
		}

	// Prior to calling Im3d::Draw we need to merge the per-thread contexts into the main thread context.
		for (int i = 0; i < g_ThreadCount; ++i) {
			auto& ctx = g_ThreadContexts[i];

			#if 1
				Im3d::Merge(Im3d::GetContext(), ctx);
			#else
			 // it's also legal to draw the context directly, however this does not preserve layer ordering and doesn't support sorting between contexts
				ctx.draw();
			#endif
		}

		example.draw(); // calls Im3d_Draw() (see im3d_opengl33.cpp).
	}
	example.shutdown();
	
	return 0;
}

void ThreadDraw(int _threadIndex)
{
	Im3d::SetContext(g_ThreadContexts[_threadIndex]);
	//Im3d::NewFrame(); // in this example we call ctx.reset() outside the thread, which is equivalent

	Im3d::RandSeed(_threadIndex); // \todo this is potentially not thread safe

	//Im3d::PushLayerId((Im3d::Id)_threadIndex);
	Im3d::PushDrawState();
	Im3d::PushEnableSorting(g_EnableSorting);
		Im3d::SetColor(g_ThreadColors[_threadIndex]);
	
		Im3d::BeginPoints();
			for (int i = 0; i < 1000; ++i) {
				Im3d::Vertex(Im3d::RandVec3(-10.0f, 10.0f), Im3d::RandFloat(2.0f, 16.0f));
			}
		Im3d::End();
		
		Im3d::SetAlpha(0.5f);
		Im3d::BeginTriangles();
			Im3d::PushMatrix();
			for (int i = 0; i < 50; ++i) {				
				Im3d::Mat4 wm(1.0f);
				wm.setRotation(Im3d::Rotation(Im3d::Normalize(Im3d::RandVec3(-1.0f, 1.0f)), Im3d::RandFloat(0.0f, 6.0f)));
				wm.setTranslation(Im3d::RandVec3(-10.0f, 10.0f));
				Im3d::SetMatrix(wm);
				Im3d::Vertex(-1.0f,  0.0f, -1.0f);
				Im3d::Vertex( 0.0f,  2.0f, -1.0f);
				Im3d::Vertex( 1.0f,  0.0f, -1.0f);
			}
			Im3d::PopMatrix();
		Im3d::End();
	Im3d::PopEnableSorting();
	Im3d::PopDrawState();
	//Im3d::PopLayerId();
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
	if (ImGui::TreeNode("Threads")) {
		ImGui::SliderInt("Thread Count", &g_ThreadCount, 0, kThreadCountMax);
		ImGui::Checkbox("Enable Sorting", &g_EnableSorting);
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

	ImGui::End();
}