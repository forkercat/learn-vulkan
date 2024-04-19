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

	class VectorFieldApp
	{
	public:
		VectorFieldApp();
		~VectorFieldApp();

		VectorFieldApp(const VectorFieldApp&) = delete;
		VectorFieldApp& operator=(const VectorFieldApp&) = delete;

		void Run();

	public:
		static constexpr U32 WIDTH = 800;
		static constexpr U32 HEIGHT = 800;

	private:
		void LoadGameObjects();

	private:
		LveWindow m_window{ WIDTH, HEIGHT, "Vector Field App" };
		LveDevice m_device{ m_window };
		LveRenderer m_renderer{ m_window, m_device };

		std::vector<LveGameObject> m_physicsGameObjects;
		std::vector<LveGameObject> m_vectorField;

		F64 m_elapsedTime;
	};

}  // namespace lve
