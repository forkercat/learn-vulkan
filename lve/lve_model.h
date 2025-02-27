//
// Created by Junhao Wang (@forkercat) on 4/13/24.
//

#pragma once

#include "core/core.h"

#include "lve_device.h"
#include "lve_buffer.h"

#include <vector>

namespace lve
{
	class LveModel
	{
	public:
		struct Vertex
		{
			Vector3 position;
			Vector3 color;
			Vector3 normal;
			Vector2 uv;

			static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

			bool operator==(const Vertex& other) const
			{
				return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
			}
		};

		struct Builder
		{
			std::vector<Vertex> vertices{};
			std::vector<U32> indices{};

			void LoadModel(const std::string& filepath);
		};

		LveModel(LveDevice& device, const Builder& builder);
		~LveModel();

		LveModel(const LveModel&) = delete;
		LveModel& operator=(const LveModel&) = delete;

		void Bind(VkCommandBuffer commandBuffer);
		void Draw(VkCommandBuffer commandBuffer);

		static UniqueRef<LveModel> CreateCubeModel(LveDevice& device, Vector3 offset);
		static UniqueRef<LveModel> CreateModelFromFile(LveDevice& device, const std::string& filepath);

	private:
		void CreateVertexBuffers(const std::vector<Vertex>& vertices);
		void CreateIndexBuffers(const std::vector<U32>& indices);

	private:
		LveDevice& m_device;

		UniqueRef<LveBuffer> m_vertexBuffer;
		U32 m_vertexCount;

		bool m_hasIndexBuffer = false;
		UniqueRef<LveBuffer> m_indexBuffer;
		U32 m_indexCount;
	};

} // namespace lve
