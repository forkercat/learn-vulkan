//
// Created by Junhao Wang (@forkercat) on 4/18/24.
//

#pragma once

#include "lve/lve_game_object.h"

namespace lve {

	class GravityPhysicsSystem
	{
	public:
		GravityPhysicsSystem(F32 strength)
			: strengthGravity(strength)
		{
		}

		// substeps is how many intervals to divide the forward time step in. More substeps result
		// in a more stable simulation, but takes longer to compute.
		void Update(std::vector<LveGameObject>& gameObjects, F32 dt, U32 substeps = 1)
		{
			const F32 stepDelta = dt / substeps;

			for (int i = 0; i < substeps; ++i)
			{
				StepSimulation(gameObjects, stepDelta);
			}
		}

		glm::vec2 ComputeForce(LveGameObject& fromGameObject, LveGameObject& toGameObject) const
		{
			glm::vec2 offset = fromGameObject.transform2d.translation - toGameObject.transform2d.translation;
			F32 distanceSquared = glm::dot(offset, offset);

			// Going to return 0 if objects are too close together.
			if (glm::abs(distanceSquared) < 1e-10f)
			{
				return { 0.0f, 0.0f };
			}

			F32 force = strengthGravity * toGameObject.rigidBody2d.mass * fromGameObject.rigidBody2d.mass / distanceSquared;
			return force * offset / glm::sqrt(distanceSquared);
		}

	private:
		void StepSimulation(std::vector<LveGameObject>& physicsObjects, F32 dt)
		{
			// Loops through all pairs of objects and applies attractive force between them.
			for (auto iterA = physicsObjects.begin(); iterA != physicsObjects.end(); ++iterA)
			{
				auto& objectA = *iterA;

				for (auto iterB = iterA; iterB != physicsObjects.end(); ++iterB)
				{
					if (iterA == iterB)
						continue;

					auto& objectB = *iterB;
					glm::vec2 force = ComputeForce(objectA, objectB);
					objectA.rigidBody2d.velocity += dt * -force / objectA.rigidBody2d.mass;
					objectB.rigidBody2d.velocity += dt * force / objectB.rigidBody2d.mass;
				}
			}

			// Update each object's position based on its final velocity.
			for (auto& object : physicsObjects)
			{
				object.transform2d.translation += dt * object.rigidBody2d.velocity;
			}
		}

	public:
		const F32 strengthGravity;
	};

}  // namespace lve
