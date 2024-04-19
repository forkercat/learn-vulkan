//
// Created by Junhao Wang (@forkercat) on 4/13/24.
//

#include "lve_model.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace lve {

	std::vector<VkVertexInputBindingDescription> LveModel::Vertex::GetBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> LveModel::Vertex::GetAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		// attributeDescriptions[2].binding = 0;
		// attributeDescriptions[2].location = 2;
		// attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		// attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}

	LveModel::LveModel(LveDevice& device, const std::vector<Vertex>& vertices)
		: m_device(device)
	{
		CreateVertexBuffers(vertices);
	}

	LveModel::~LveModel()
	{
		vkDestroyBuffer(m_device.GetDevice(), m_vertexBuffer, nullptr);
		vkFreeMemory(m_device.GetDevice(), m_vertexBufferMemory, nullptr);
	}

	void LveModel::Bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { m_vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	}

	void LveModel::Draw(VkCommandBuffer commandBuffer)
	{
		vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);
	}

	void LveModel::CreateVertexBuffers(const std::vector<Vertex>& vertices)
	{
		m_vertexCount = static_cast<U32>(vertices.size());
		ASSERT(m_vertexCount >= 3, "Failed to create vertex buffer. Vertex count must be at least 3!");

		VkDeviceSize bufferSize = sizeof(vertices[0]) * m_vertexCount;

		m_device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
							  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_vertexBuffer,
							  m_vertexBufferMemory);

		// Map the host memory on CPU to the device memory on GPU.
		void* data;
		vkMapMemory(m_device.GetDevice(), m_vertexBufferMemory, 0, bufferSize, 0, &data);
		// memcpy will copy the vertices data memory to the memory on CPU.
		// Since we use COHERENT flag, the CPU memory will automatically be flushed to update GPU memory.
		memcpy(data, vertices.data(), static_cast<USize>(bufferSize));
		vkUnmapMemory(m_device.GetDevice(), m_vertexBufferMemory);
	}

	UniqueRef<LveModel> LveModel::CreateSquareModel(LveDevice& device, glm::vec2 offset)
	{
		std::vector<Vertex> vertices = { { { -0.5f, -0.5f } }, { { 0.5f, 0.5f } },	{ { -0.5f, 0.5f } },
										 { { -0.5f, -0.5f } }, { { 0.5f, -0.5f } }, { { 0.5f, 0.5f } } };

		for (auto& v : vertices)
		{
			v.position += offset;
		}

		return MakeUniqueRef<LveModel>(device, vertices);
	}

	UniqueRef<LveModel> LveModel::CreateCircleModel(LveDevice& device, U32 numSides)
	{
		std::vector<Vertex> uniqueVertices{};
		for (int i = 0; i < numSides; i++)
		{
			F32 angle = i * glm::two_pi<F32>() / numSides;
			uniqueVertices.push_back({ { glm::cos(angle), glm::sin(angle) } });
		}

		uniqueVertices.push_back({}); // adds center vertex at 0, 0

		// Makes triangles within the circle.
		std::vector<Vertex> vertices{};
		for (int i = 0; i < numSides; i++)
		{
			vertices.push_back(uniqueVertices[i]);
			vertices.push_back(uniqueVertices[(i + 1) % numSides]);
			vertices.push_back(uniqueVertices[numSides]);
		}

		return MakeUniqueRef<LveModel>(device, vertices);
	}

}  // namespace lve
