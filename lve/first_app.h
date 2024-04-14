//
// Created by Junhao Wang (@forkercat) on 4/1/24.
//

#pragma once

#include "core/core.h"

#include "lve_window.h"
#include "lve_device.h"
#include "lve_renderer.h"
#include "lve_game_object.h"

#include <vector>
#include <memory>

namespace lve {

	class FirstApp
	{
	public:
		FirstApp();
		~FirstApp();

		FirstApp(const FirstApp&) = delete;
		FirstApp& operator=(const FirstApp&) = delete;

		void Run();

	public:
		static constexpr U32 sWidth = 800;
		static constexpr U32 sHeight = 600;

	private:
		void LoadGameObjects();

	private:
		LveWindow mWindow{ sWidth, sHeight, "Hello Vulkan!" };
		LveDevice mDevice{ mWindow };
		LveRenderer mRenderer{ mWindow, mDevice };

		std::vector<LveGameObject> mGameObjects;
	};

}  // namespace lve
