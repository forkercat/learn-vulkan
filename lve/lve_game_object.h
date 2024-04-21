//
// Created by Junhao Wang (@forkercat) on 4/14/24.
//

#pragma once

#include "core/core.h"

#include "lve_model.h"

#include <memory>

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

		// Matrix corresponds to Translate * Ry * Rx * Rz * Scale
		// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
		// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		Matrix4 GetTransform()
		{
			const float c3 = MathOp::Cos(rotation.z);
			const float s3 = MathOp::Sin(rotation.z);
			const float c2 = MathOp::Cos(rotation.x);
			const float s2 = MathOp::Sin(rotation.x);
			const float c1 = MathOp::Cos(rotation.y);
			const float s1 = MathOp::Sin(rotation.y);
			return Matrix4{
				{
					scale.x * (c1 * c3 + s1 * s2 * s3),
					scale.x * (c2 * s3),
					scale.x * (c1 * s2 * s3 - c3 * s1),
					0.0f,
				},
				{
					scale.y * (c3 * s1 * s2 - c1 * s3),
					scale.y * (c2 * c3),
					scale.y * (c1 * c3 * s2 + s1 * s3),
					0.0f,
				},
				{
					scale.z * (c2 * s1),
					scale.z * (-s2),
					scale.z * (c1 * c2),
					0.0f,
				},
				{ translation.x, translation.y, translation.z, 1.0f }
			};
		}
	};

	/////////////////////////////////////////////////////////////////////////////////
	// LveGameObject
	/////////////////////////////////////////////////////////////////////////////////

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
