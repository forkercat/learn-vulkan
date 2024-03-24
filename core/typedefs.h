//
// Created by Junhao Wang (@forkercat) on 3/2/24.
//

#pragma once

#include <cstdint>
#include <memory>

// Integers
using U64 = std::uint64_t;
using U32 = std::uint32_t;
using U16 = std::uint16_t;
using U8 = std::uint8_t;

using I64 = std::int64_t;
using I32 = std::int32_t;
using I16 = std::int16_t;
using I8 = std::int8_t;

using USize = size_t;

// Floats
#ifdef FORCE_FLOAT_64
using F32 = double;
#else
using F32 = float;
#endif
using F64 = double;

// Math
#include <glm/glm.hpp>
using Vec1 = glm::vec1;
using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;

using Mat2x2 = glm::mat2x2;
using Mat3x3 = glm::mat3x3;
using Mat4x4 = glm::mat4x4;

using Mat2x3 = glm::mat2x3;
using Mat2x4 = glm::mat2x4;
using Mat3x4 = glm::mat3x4;
using Mat4x3 = glm::mat4x3;
using Mat4x2 = glm::mat4x2;
using Mat3x2 = glm::mat3x2;

// Reference
template <typename T>
using Scope = std::unique_ptr<T>;

template <typename T, typename... Args>
constexpr Scope<T> CreateScope(Args&&... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T>
using Ref = std::shared_ptr<T>;

template <typename T, typename... Args>
constexpr Ref<T> CreateRef(Args&&... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

#define BIT(x) (1 << x)
#define STR(x) #x
