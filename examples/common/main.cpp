#include "im3d_example.h"

using namespace Im3d;

int main(int, char**)
{
	Example example;
	if (!example.init(-1, -1, "Im3d Example")) {
		return 1;
	}

	while (example.update()) {

		
		ImGui::ShowTestWindow();

		example.draw();
	}
	example.shutdown();
	
	return 0;
}
