/*	Multi context example
	This example shows how to use multiple Im3d contexts to draw from multiple threads. 

	Im3d doesn't provide any thread safety mechanism per se; applications are expected to 
	draw to per-thread contexts and then use Im3d::MergeContexts() to combine vertex data 
	and draw on the main thread.

	There are some prerequisites to making this work:

	1) #define IM3D_THREAD_LOCAL_CONTEXT_PTR 1 - either modify im3d_config.h or define via the build
	system. This is required to declare Im3d's internal context ptr as thread_local which allows the
	application to set a different context per thread.

	2) Declare some number of Im3d::Context instances (1 per thread) - the internal context ptr is thread
	local but points to the main context by default. Im3d::SetContext(&threadCtx) must therefore be called 
	on each thread.

	3) At the beginning of the frame, fill the Im3d::AppData struct for *all* contexts which will be used 
	during the frame. The simplest approach is to fill this once on the main thread and then copy the
	result into each per-thread context. See the integration examples for how to fill the AppData struct.

	4) Towards the end of the frame, merge each per-thread context into the main thread via Im3d::MergeContexts(), 
	then call Im3d::EndFrame() and draw the combined draw lists. This requires synchronization to ensure that 
	threads cannot modify either context during the merge.
*/
#include "im3d_example.h"

#include <thread>

static const int     kThreadCountMax  = 6;
static int           g_ThreadCount    = kThreadCountMax;
static Im3d::Context g_ThreadContexts[kThreadCountMax];
static bool          g_EnableSorting  = false;
static Im3d::Mat4    g_ThreadGizmoTest[kThreadCountMax];
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
	if (!example.init(-1, -1, "Im3d Example"))
	{
		return 1;
	}

	for (int i = 0; i < kThreadCountMax; ++i)
	{
		float threadX = (float)i / (float)kThreadCountMax * 10.0f - 5.0f;
		g_ThreadGizmoTest[i] = Im3d::Mat4(Im3d::Vec3(threadX, 0.0f, 0.0f), Im3d::Mat3(1.0f), Im3d::Vec3(1.0f));
	}

	while (example.update()) // calls Im3d_NewFrame() (see im3d_opengl33.cpp)
	{
	// At this point we have updated the default context and filled its AppData struct. 

	// Each separate context could potentially use different AppData (e.g. different cameras/viewports). Here 
	// we just copy the default for simplicity.
		for (auto& ctx : g_ThreadContexts)
		{
			ctx.getAppData() = Im3d::GetAppData();

			ctx.reset(); // equivalent to calling Im3d::NewFrame() on the thread
		}

		MainThreadDraw(); 

	// The frame can now proceed, threads can safely make Im3d:: calls (after setting the context ptr, see ThreadDraw()).
		std::thread threads[kThreadCountMax];
		for (int i = 0; i < g_ThreadCount; ++i)
		{
			threads[i] = std::thread(&ThreadDraw, i);
		}		

	// Towards the end of the frame we need to synchronize so that we can safely merge vertex data from the per-thread contexts.
		for (int i = 0; i < g_ThreadCount; ++i)
		{
			threads[i].join();
		}

	// Prior to calling Im3d::EndFrame() we need to merge the per-thread contexts into the main thread context.
		for (int i = 0; i < g_ThreadCount; ++i)
		{
			Im3d::MergeContexts(Im3d::GetContext(), g_ThreadContexts[i]);
		}

		example.draw(); // calls Im3d_EndFrame() (see im3d_opengl33.cpp).
	}
	example.shutdown();
	
	return 0;
}

void ThreadDraw(int _threadIndex)
{
	Im3d::SetContext(g_ThreadContexts[_threadIndex]);
	//Im3d::NewFrame(); // in this example we call ctx.reset() outside the thread, which is equivalent
	
 // Gizmos work, however the application is responsible for isolating inputs between multiple contexts.
 // In this example we simply copy AppData from the main thread, therefore it's possible to interact with
 // multiple gizmos simultaneously.
	Im3d::Gizmo("Gizmo", (float*)&g_ThreadGizmoTest[_threadIndex]);

	Im3d::PushMatrix(g_ThreadGizmoTest[_threadIndex]);

	#if 0
		Im3d::PushDrawState();
		Im3d::PushEnableSorting(g_EnableSorting);
			Im3d::SetColor(g_ThreadColors[_threadIndex]);
			Im3d::BeginTriangles();
				Im3d::Vertex(-1.0f,  0.0f, 0.0f);
				Im3d::Vertex( 0.0f,  2.0f, 0.0f);
				Im3d::Vertex( 1.0f,  0.0f, 0.0f);
			Im3d::End();
		Im3d::PopEnableSorting();
		Im3d::PopDrawState();
	#endif

	Im3d::Text(Im3d::Vec3(0.0f, 1.0f, 0.0f), 2.0f, Im3d::Color_White, 0, "Thread %d", _threadIndex);

	Im3d::RandSeed(_threadIndex); // \todo this is potentially not thread safe

	Im3d::PushDrawState();
	Im3d::PushEnableSorting(g_EnableSorting);
		Im3d::SetColor(g_ThreadColors[_threadIndex]);
	
		Im3d::BeginPoints();
			for (int i = 0; i < 256; ++i)
			{
				Im3d::Vertex(Im3d::RandVec3(-10.0f, 10.0f), Im3d::RandFloat(2.0f, 16.0f));
			}
		Im3d::End();
		
		Im3d::SetAlpha(0.5f);
		Im3d::BeginTriangles();
			for (int i = 0; i < 32; ++i)
			{
				Im3d::Mat4 wm(1.0f);
				Im3d::PushMatrix();
					Im3d::Rotate(Im3d::Normalize(Im3d::RandVec3(-1.0f, 1.0f)), Im3d::RandFloat(0.0f, 6.0f));
					Im3d::Translate(Im3d::RandVec3(-10.0f, 10.0f));
					Im3d::Vertex(-1.0f,  0.0f, -1.0f);
					Im3d::Vertex( 0.0f,  2.0f, -1.0f);
					Im3d::Vertex( 1.0f,  0.0f, -1.0f);
				Im3d::PopMatrix();
			}
		Im3d::End();

	Im3d::PopEnableSorting();
	Im3d::PopDrawState();
	Im3d::PopMatrix();
}

void MainThreadDraw()
{
	Im3d::Context& ctx = Im3d::GetContext();
	Im3d::AppData& ad  = Im3d::GetAppData();

	ImGui::Begin("Im3d Demo", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_Once);
	if (ImGui::TreeNode("About"))
	{
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
	if (ImGui::TreeNode("Threads"))
	{
		ImGui::SliderInt("Thread Count", &g_ThreadCount, 0, kThreadCountMax);
		ImGui::Checkbox("Enable Sorting", &g_EnableSorting);
		ImGui::TreePop();
	}

	ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_Once);
	if (ImGui::TreeNode("Grid"))
	{
		static int gridSize = 20;
		ImGui::SliderInt("Grid Size", &gridSize, 1, 50);
		const float gridHalf = (float)gridSize * 0.5f;
		Im3d::SetAlpha(1.0f);
		Im3d::SetSize(1.0f);
		Im3d::BeginLines();
			for (int x = 0; x <= gridSize; ++x)
			{
				Im3d::Vertex(-gridHalf, 0.0f, (float)x - gridHalf, Im3d::Color(0.0f, 0.0f, 0.0f));
				Im3d::Vertex( gridHalf, 0.0f, (float)x - gridHalf, Im3d::Color(1.0f, 0.0f, 0.0f));
			}
			for (int z = 0; z <= gridSize; ++z)
			{
				Im3d::Vertex((float)z - gridHalf, 0.0f, -gridHalf,  Im3d::Color(0.0f, 0.0f, 0.0f));
				Im3d::Vertex((float)z - gridHalf, 0.0f,  gridHalf,  Im3d::Color(0.0f, 0.0f, 1.0f));
			}
		Im3d::End();

		ImGui::TreePop();
	}

	ImGui::End();
}