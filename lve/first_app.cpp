//
// Created by Junhao Wang (@forkercat) on 4/1/24.
//

#include "first_app.h"

namespace lve {

	void FirstApp::Run()
	{
		while (!mWindow.ShouldClose())
		{
			glfwPollEvents();
		}
	}

}  // namespace lve
