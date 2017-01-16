#include "common/common.h"

using namespace Im3d;

int main(int, char**)
{
	if (!Example_Init(-1, -1, "Im3d Example")) {
		return 1;
	}

	while (Example_update()) {

		Example_Draw();
	}
	Example_Shutdown();
	
	return 0;
}
