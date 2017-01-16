#include "im3d_example.h"

using namespace Im3d;

int main(int, char**)
{
	Example example;
	if (!example.init(-1, -1, "Im3d Example")) {
		return 1;
	}

	while (example.update()) {
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
		
			Im3d::BeginTriangles();
				Im3d::Vertex(-1.0f,  0.0f, -1.0f, Im3d::Color_Red);
				Im3d::Vertex( 0.0f,  2.0f, -1.0f, Im3d::Color_Green);
				Im3d::Vertex( 1.0f,  0.0f, -1.0f, Im3d::Color_Blue);
			Im3d::End();
			Im3d::SetSize(2.0f);
			Im3d::BeginLineLoop();
				Im3d::Vertex(-1.0f,  0.0f, -1.0f, Im3d::Color_Magenta);
				Im3d::Vertex( 0.0f,  2.0f, -1.0f, Im3d::Color_Yellow);
				Im3d::Vertex( 1.0f,  0.0f, -1.0f, Im3d::Color_Cyan);
			Im3d::End();

		Im3d::PopDrawState();


		example.draw();
	}
	example.shutdown();
	
	return 0;
}
