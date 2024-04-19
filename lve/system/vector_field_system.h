//
// Created by Junhao Wang (@forkercat) on 4/18/24.
//

#pragma once

#include "gravity_physics_system.h"

#include "lve/lve_game_object.h"

#include <vector>

namespace lve {

	class VectorFieldSystem
	{
	public:
		void Update(const GravityPhysicsSystem& physicsSystem, std::vector<LveGameObject>& physicsObjects,
					std::vector<LveGameObject>& vectorField)
		{
			// For each line vf we calculate the net graviation force for that point in space.
			for (auto& vf : vectorField)
			{
				glm::vec2 direction{};

				for (auto& obj : physicsObjects)
				{
					direction += physicsSystem.ComputeForce(obj, vf);
				}

				// This scales the length of the field line based on the log of the length.
				// Values were chosen just through trial and error based on what I liked the look
				// of and then the field line is rotated to point in the direction of the field.
				vf.transform2d.scale.x = 0.005f + 0.045 * glm::clamp(glm::log(glm::length(direction) + 1) / 3.0f, 0.f, 1.0f);
				vf.transform2d.rotation = atan2(direction.y, direction.x);
			}
		}
	};

}  // namespace lve
