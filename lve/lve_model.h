//
// Created by Junhao Wang (@forkercat) on 4/13/24.
//

#pragma once

#include "core/core.h"

#include "lve_device.h"

#include <vector>

namespace lve {

	class LveModel
	{
	public:
		struct Vertex
		{
			Vector3 position;
			Vector3 color;

			static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
		};

		LveModel(LveDevice& device, const std::vector<Vertex>& vertices);
		~LveModel();

		LveModel(const LveModel&) = delete;
		LveModel& operator=(const LveModel&) = delete;

		void Bind(VkCommandBuffer commandBuffer);
		void Draw(VkCommandBuffer commandBuffer);

		static UniqueRef<LveModel> CreateCubeModel(LveDevice& device, Vector3 offset);

	private:
		void CreateVertexBuffers(const std::vector<Vertex>& vertices);

	private:
		LveDevice& m_device;
		VkBuffer m_vertexBuffer;
		VkDeviceMemory m_vertexBufferMemory;
		U32 m_vertexCount;
	};

}  // namespace lve
