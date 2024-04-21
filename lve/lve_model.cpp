//
// Created by Junhao Wang (@forkercat) on 4/13/24.
//

#include "lve_model.h"

#include "lve_utils.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace std
{
	template <>
	struct hash<lve::LveModel::Vertex>
	{
		size_t operator()(const lve::LveModel::Vertex& vertex) const
		{
			size_t seed = 0;
			lve::HashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
			return seed;
		}
	};

} // namespace std

namespace lve
{
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
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
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

	LveModel::LveModel(LveDevice& device, const Builder& builder)
		: m_device(device)
	{
		CreateVertexBuffers(builder.vertices);
		CreateIndexBuffers(builder.indices);
	}

	LveModel::~LveModel()
	{
		vkDestroyBuffer(m_device.GetDevice(), m_vertexBuffer, nullptr);
		vkFreeMemory(m_device.GetDevice(), m_vertexBufferMemory, nullptr);

		if (m_hasIndexBuffer)
		{
			vkDestroyBuffer(m_device.GetDevice(), m_indexBuffer, nullptr);
			vkFreeMemory(m_device.GetDevice(), m_indexBufferMemory, nullptr);
		}
	}

	void LveModel::Bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { m_vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

		if (m_hasIndexBuffer)
		{
			vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		}
	}

	void LveModel::Draw(VkCommandBuffer commandBuffer)
	{
		if (m_hasIndexBuffer)
		{
			vkCmdDrawIndexed(commandBuffer, m_indexCount, 1, 0, 0, 0);
		}
		else
		{
			vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);
		}
	}

	void LveModel::CreateVertexBuffers(const std::vector<Vertex>& vertices)
	{
		m_vertexCount = static_cast<U32>(vertices.size());
		ASSERT(m_vertexCount >= 3, "Failed to create vertex buffer. Vertex count must be at least 3!");

		VkDeviceSize bufferSize = sizeof(vertices[0]) * m_vertexCount;

		// Create staging buffer.
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		m_device.CreateBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory);

		// Map the host memory on CPU to the device memory on GPU.
		void* data;
		vkMapMemory(m_device.GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		// memcpy will copy the vertices data memory to the memory on CPU.
		// Since we use COHERENT flag, the CPU memory will automatically be flushed to update GPU memory.
		memcpy(data, vertices.data(), static_cast<USize>(bufferSize));
		vkUnmapMemory(m_device.GetDevice(), stagingBufferMemory);

		// Create vertex buffer.
		m_device.CreateBuffer(
			bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_vertexBuffer,
			m_vertexBufferMemory);

		// Copy staging to vertex buffer.
		m_device.CopyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

		vkDestroyBuffer(m_device.GetDevice(), stagingBuffer, nullptr);
		vkFreeMemory(m_device.GetDevice(), stagingBufferMemory, nullptr);
	}

	void LveModel::CreateIndexBuffers(const std::vector<U32>& indices)
	{
		m_indexCount = static_cast<U32>(indices.size());
		m_hasIndexBuffer = m_indexCount > 0;

		if (!m_hasIndexBuffer)
		{
			return;
		}

		VkDeviceSize bufferSize = sizeof(indices[0]) * m_indexCount;

		// Create staging buffer.
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		m_device.CreateBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory);

		// Map the host memory on CPU to the device memory on GPU.
		void* data;
		vkMapMemory(m_device.GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), static_cast<USize>(bufferSize));
		vkUnmapMemory(m_device.GetDevice(), stagingBufferMemory);

		// Create index buffer.
		m_device.CreateBuffer(
			bufferSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_indexBuffer,
			m_indexBufferMemory);

		// Copy staging to index buffer.
		m_device.CopyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

		vkDestroyBuffer(m_device.GetDevice(), stagingBuffer, nullptr);
		vkFreeMemory(m_device.GetDevice(), stagingBufferMemory, nullptr);
	}

	UniqueRef<LveModel> LveModel::CreateCubeModel(LveDevice& device, Vector3 offset)
	{
		// temporary helper function, creates a 1x1x1 cube centered at offset
		Builder modelBuilder{};

		modelBuilder.vertices = {
			// left face (white)
			{ { -.5f, -.5f, -.5f }, { .9f, .9f, .9f } },
			{ { -.5f, .5f, .5f }, { .9f, .9f, .9f } },
			{ { -.5f, -.5f, .5f }, { .9f, .9f, .9f } },
			{ { -.5f, .5f, -.5f }, { .9f, .9f, .9f } },

			// right face (yellow)
			{ { .5f, -.5f, -.5f }, { .8f, .8f, .1f } },
			{ { .5f, .5f, .5f }, { .8f, .8f, .1f } },
			{ { .5f, -.5f, .5f }, { .8f, .8f, .1f } },
			{ { .5f, .5f, -.5f }, { .8f, .8f, .1f } },

			// top face (orange, remember y-axis points down)
			{ { -.5f, -.5f, -.5f }, { .9f, .6f, .1f } },
			{ { .5f, -.5f, .5f }, { .9f, .6f, .1f } },
			{ { -.5f, -.5f, .5f }, { .9f, .6f, .1f } },
			{ { .5f, -.5f, -.5f }, { .9f, .6f, .1f } },

			// bottom face (red)
			{ { -.5f, .5f, -.5f }, { .8f, .1f, .1f } },
			{ { .5f, .5f, .5f }, { .8f, .1f, .1f } },
			{ { -.5f, .5f, .5f }, { .8f, .1f, .1f } },
			{ { .5f, .5f, -.5f }, { .8f, .1f, .1f } },

			// nose face (blue)
			{ { -.5f, -.5f, 0.5f }, { .1f, .1f, .8f } },
			{ { .5f, .5f, 0.5f }, { .1f, .1f, .8f } },
			{ { -.5f, .5f, 0.5f }, { .1f, .1f, .8f } },
			{ { .5f, -.5f, 0.5f }, { .1f, .1f, .8f } },

			// tail face (green)
			{ { -.5f, -.5f, -0.5f }, { .1f, .8f, .1f } },
			{ { .5f, .5f, -0.5f }, { .1f, .8f, .1f } },
			{ { -.5f, .5f, -0.5f }, { .1f, .8f, .1f } },
			{ { .5f, -.5f, -0.5f }, { .1f, .8f, .1f } },
		};

		for (auto& v : modelBuilder.vertices)
		{
			v.position += offset;
		}

		modelBuilder.indices = {
			0, 1, 2, 0, 3, 1, 4, 5, 6, 4, 7, 5, 8, 9, 10, 8, 11, 9, 12, 13,
			14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21
		};

		return MakeUniqueRef<LveModel>(device, modelBuilder);
	}

	UniqueRef<LveModel> LveModel::CreateModelFromFile(LveDevice& device, const std::string& filepath)
	{
		Builder builder{};
		builder.LoadModel(filepath);
		PRINT("Vertex count: %zu", builder.vertices.size());
		return MakeUniqueRef<LveModel>(device, builder);
	}

	void LveModel::Builder::LoadModel(const std::string& filepath)
	{
		using tinyobj::attrib_t;
		using tinyobj::index_t;
		using tinyobj::material_t;
		using tinyobj::shape_t;

		attrib_t attrib;
		std::vector<shape_t> shapes;
		std::vector<material_t> materials;
		std::string warn, error;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &error, filepath.c_str()))
		{
			ASSERT(false, "Failed to load model: %s", filepath.c_str());
		}

		vertices.clear();
		indices.clear();

		// [Vertex: Index]
		std::unordered_map<Vertex, U32> uniqueVertices{};

		for (const shape_t& shape : shapes)
		{
			for (const index_t& index : shape.mesh.indices)
			{
				Vertex vertex{};

				if (index.vertex_index >= 0)
				{
					vertex.position = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					};

					int colorIndex = 3 * index.vertex_index + 2;

					if (colorIndex < attrib.colors.size()) // check if color index is in bound
					{
						vertex.color = {
							attrib.colors[colorIndex - 2],
							attrib.colors[colorIndex - 1],
							attrib.colors[colorIndex - 0]
						};
					}
					else
					{
						vertex.color = { 1.0f, 1.0f, 1.0f }; // set the default color
					}
				}

				if (index.normal_index >= 0)
				{
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					};
				}

				if (index.texcoord_index >= 0)
				{
					vertex.uv = {
						attrib.texcoords[3 * index.texcoord_index + 0],
						attrib.texcoords[3 * index.texcoord_index + 1]
					};
				}

				// Encounter new vertex. Only add to vertices when it is not yet added.
				if (uniqueVertices.count(vertex) == 0)
				{
					U32 vertexIndex = static_cast<U32>(vertices.size());
					uniqueVertices[vertex] = vertexIndex;
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}
	}

} // namespace lve
