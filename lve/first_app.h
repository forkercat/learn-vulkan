//
// Created by Junhao Wang (@forkercat) on 4/1/24.
//

#pragma once

#include "lve_window.h"

#include "core/core.h"

namespace lve {

	class FirstApp
	{
	public:
		void Run();

	public:
		static constexpr U32 kWidth = 800;
		static constexpr U32 kHeight = 600;

	private:
		LveWindow mWindow{ kWidth, kHeight, "Hello Vulkan!" };
	};

}  // namespace lve
