//
// Created by Junhao Wang (@forkercat) on 4/14/24.
//

#pragma once

#include "core/core.h"

#include "lve_model.h"

#include <memory>

namespace lve {

	struct Transform2dComponent
	{
		glm::vec2 translation{};
		glm::vec2 scale{ 1.f, 1.f };
		F32 rotation;

		glm::mat2 GetTransform()
		{
			const F32 sin = glm::sin(rotation);
			const F32 cos = glm::cos(rotation);
			glm::mat2 rotationMatrix({ cos, sin }, { -sin, cos });
			glm::mat2 scaleMatrix({ scale.x, 0.f }, { 0.f, scale.y });

			return rotationMatrix * scaleMatrix;
		}
	};

	class LveGameObject
	{
	public:
		using id_t = unsigned int;

	public:
		LveGameObject(const LveGameObject&) = delete;
		LveGameObject& operator=(const LveGameObject&) = delete;
		LveGameObject(LveGameObject&&) = default;
		LveGameObject& operator=(LveGameObject&&) = default;

		static LveGameObject CreateGameObject()
		{
			static id_t currentId = 0;
			return LveGameObject(currentId++);
		}

		id_t GetId() { return mId; }

	public:
		std::shared_ptr<LveModel> model{};
		glm::vec3 color{};
		Transform2dComponent transform2d;

	private:
		LveGameObject(id_t objectId)
			: mId(objectId)
		{
		}

	private:
		id_t mId;
	};

}  // namespace lve
