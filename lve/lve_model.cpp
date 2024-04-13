//
// Created by Junhao Wang (@forkercat) on 4/13/24.
//

#include "lve_model.h"

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
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		// attributeDescriptions[1].binding = 0;
		// attributeDescriptions[1].location = 1;
		// attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		// attributeDescriptions[1].offset = offsetof(Vertex, color);
		//
		// attributeDescriptions[2].binding = 0;
		// attributeDescriptions[2].location = 2;
		// attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		// attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}

	LveModel::LveModel(LveDevice& device, const std::vector<Vertex>& vertices)
		: mDevice(device)
	{
		CreateVertexBuffers(vertices);
	}

	LveModel::~LveModel()
	{
		vkDestroyBuffer(mDevice.GetDevice(), mVertexBuffer, nullptr);
		vkFreeMemory(mDevice.GetDevice(), mVertexBufferMemory, nullptr);
	}

	void LveModel::Bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { mVertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	}

	void LveModel::Draw(VkCommandBuffer commandBuffer)
	{
		vkCmdDraw(commandBuffer, mVertexCount, 1, 0, 0);
	}

	void LveModel::CreateVertexBuffers(const std::vector<Vertex>& vertices)
	{
		mVertexCount = static_cast<U32>(vertices.size());
		ASSERT(mVertexCount >= 3, "Failed to create vertex buffer. Vertex count must be at least 3!");

		VkDeviceSize bufferSize = sizeof(vertices[0]) * mVertexCount;

		mDevice.CreateBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
							 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mVertexBuffer,
							 mVertexBufferMemory);

		// Map the host memory on CPU to the device memory on GPU.
		void* data;
		vkMapMemory(mDevice.GetDevice(), mVertexBufferMemory, 0, bufferSize, 0, &data);
		// memcpy will copy the vertices data memory to the memory on CPU.
		// Since we use COHERENT flag, the CPU memory will automatically be flushed to update GPU memory.
		memcpy(data, vertices.data(), static_cast<USize>(bufferSize));
		vkUnmapMemory(mDevice.GetDevice(), mVertexBufferMemory);
	}

}  // namespace lve
