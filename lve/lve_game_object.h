//
// Created by Junhao Wang (@forkercat) on 4/14/24.
//

#pragma once

#include "core/core.h"

#include "lve_model.h"

#include <memory>
#include <unordered_map>

namespace lve
{
	/////////////////////////////////////////////////////////////////////////////////
	// Components
	/////////////////////////////////////////////////////////////////////////////////

	struct TransformComponent
	{
		Vector3 translation{};
		Vector3 rotation{};
		Vector3 scale{ 1.0f, 1.0f, 1.0f };

		Matrix4 GetTransform();

		Matrix3 GetNormalMatrix();
	};

	/////////////////////////////////////////////////////////////////////////////////
	// LveGameObject
	/////////////////////////////////////////////////////////////////////////////////

	class LveGameObject
	{
	public:
		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, LveGameObject>;

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

		id_t GetId() { return m_id; }

	public:
		Ref<LveModel> model{};
		Vector3 color{};

		// Components
		TransformComponent transform;

	private:
		LveGameObject(id_t objectId)
			: m_id(objectId)
		{
		}

	private:
		id_t m_id;
	};

} // namespace lve
