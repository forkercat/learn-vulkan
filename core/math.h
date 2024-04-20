//
// Created by Junhao Wang (@forkercat) on 3/24/24.
//

#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

// Type definitions

using Vector1 = glm::vec1;
using Vector2 = glm::vec2;
using Vector3 = glm::vec3;
using Vector4 = glm::vec4;

using Matrix2 = glm::mat2x2;
using Matrix3 = glm::mat3x3;
using Matrix4 = glm::mat4x4;
// using Matrix2x2 = glm::mat2x2;
// using Matrix3x3 = glm::mat3x3;
// using Matrix4x4 = glm::mat4x4;

using Matrix2x3 = glm::mat2x3;
using Matrix2x4 = glm::mat2x4;
using Matrix3x4 = glm::mat3x4;
using Matrix4x3 = glm::mat4x3;
using Matrix4x2 = glm::mat4x2;
using Matrix3x2 = glm::mat3x2;

#define GLM_PI glm::pi<F32>()

// Math operations

namespace MathOp {

	inline F32 Radians(F32 degrees)
	{
		return glm::radians(degrees);
	}

	inline F32 Degrees(F32 radians)
	{
		return glm::degrees(radians);
	}

	inline F32 Abs(F32 value)
	{
		return glm::abs(value);
	}

	inline F32 Sin(F32 value)
	{
		return glm::sin(value);
	}

	inline F32 Cos(F32 value)
	{
		return glm::cos(value);
	}

	inline Vector3 Normalize(const Vector3& value)
	{
		return glm::normalize(value);
	}

	inline F32 Dot(const Vector3& v1, const Vector3& v2)
	{
		return glm::dot(v1, v2);
	}

	inline F32 Dot(const Vector2& v1, const Vector2& v2)
	{
		return glm::dot(v1, v2);
	}

	inline Vector3 Cross(const Vector3& v1, const Vector3& v2)
	{
		return glm::cross(v1, v2);
	}

}  // namespace MathOp
