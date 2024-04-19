//
// Created by Junhao Wang (@forkercat) on 4/1/24.
//

#include "vector_field_app.h"

#include "lve/system/simple_render_system.h"
#include "lve/system/rainbow_system.h"
#include "lve/system/gravity_physics_system.h"
#include "lve/system/vector_field_system.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace lve {

	VectorFieldApp::VectorFieldApp()
		: m_elapsedTime(0.0f)
	{
		LoadGameObjects();
	}

	VectorFieldApp::~VectorFieldApp()
	{
	}

	void VectorFieldApp::Run()
	{
		// Create physics system and vector field.
		GravityPhysicsSystem gravitySystem{ 0.81f };
		VectorFieldSystem vectorFieldSystem{};

		SimpleRenderSystem simpleRenderSystem(m_device, m_renderer.GetSwapchainRenderPass());
		// RainbowSystem rainbowSystem(0.4f);

		while (!m_window.ShouldClose())
		{
			// Update time.
			F64 newTime = glfwGetTime();
			// F32 deltaTime = static_cast<F32>(newTime - m_elapsedTime);
			m_elapsedTime = newTime;

			glfwPollEvents();

			// Could be nullptr if, for example, the swapchain needs to be recreated.
			if (VkCommandBuffer commandBuffer = m_renderer.BeginFrame())
			{
				// Update systems.
				gravitySystem.Update(m_physicsGameObjects, 1.0f / 60.0f, 5);
				vectorFieldSystem.Update(gravitySystem, m_physicsGameObjects, m_vectorField);

				m_renderer.BeginSwapchainRenderPass(commandBuffer);

				simpleRenderSystem.RenderGameObjects(commandBuffer, m_physicsGameObjects);
				simpleRenderSystem.RenderGameObjects(commandBuffer, m_vectorField);

				m_renderer.EndSwapchainRenderPass(commandBuffer);
				m_renderer.EndFrame();
			}
		}

		vkDeviceWaitIdle(m_device.GetDevice());
	}

	void VectorFieldApp::LoadGameObjects()
	{
		// Create some models. Offset by .5 so rotation occurs at edge rather than center of square.
		Ref<LveModel> squareModel = LveModel::CreateSquareModel(m_device, { 0.5f, 0.0f });
		Ref<LveModel> circleModel = LveModel::CreateCircleModel(m_device, { 64 });

		// Create physics objects.
		LveGameObject orangeGameObject = LveGameObject::CreateGameObject();
		orangeGameObject.transform2d.scale = glm::vec2{ 0.05f };
		orangeGameObject.transform2d.translation = { 0.5f, 0.5f };
		orangeGameObject.color = { 1.0f, 0.557f, 0.0f };
		orangeGameObject.model = circleModel;
		orangeGameObject.rigidBody2d.velocity = { -0.5f, 0.0f };

		LveGameObject blueGameObject = LveGameObject::CreateGameObject();
		blueGameObject.transform2d.scale = glm::vec2{ 0.05f };
		blueGameObject.transform2d.translation = { -0.45f, -0.25f };
		blueGameObject.color = { 0.0f, 0.247f, 0.49f };
		blueGameObject.model = circleModel;
		blueGameObject.rigidBody2d.velocity = { 0.5f, 0.0f };

		m_physicsGameObjects.push_back(std::move(orangeGameObject));
		m_physicsGameObjects.push_back(std::move(blueGameObject));

		// Create vector field objects.
		int gridCount = 40;

		for (int i = 0; i < gridCount; ++i)
		{
			for (int j = 0; j < gridCount; ++j)
			{
				LveGameObject vf = LveGameObject::CreateGameObject();
				vf.transform2d.scale = glm::vec2(0.005f);
				vf.transform2d.translation = { -1.0f + (i + 0.5f) * 2.0f / gridCount, -1.0f + (j + 0.5f) * 2.0f / gridCount };

				vf.color = glm::vec3(1.0f);
				vf.model = squareModel;
				m_vectorField.push_back(std::move(vf));
			}
		}
	}

}  // namespace lve
