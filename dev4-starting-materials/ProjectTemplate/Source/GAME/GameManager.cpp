#include "GameComponents.h"
#include "../../entt-3.13.1/single_include/entt/entt.hpp"
#include "..\CCL.h"	
#include "../DRAW/DrawComponents.h"
#include "../UTIL/Utilities.h"

namespace GAME {

	void LogCollision(GW::MATH::GCollision::GCollisionCheck collision)
	{
		switch (collision)
		{
		case GW::MATH::GCollision::GCollisionCheck::ERROR_NO_RESULT:
			std::cout << ("Error: No collision result available.") << std::endl;
			break;
		case GW::MATH::GCollision::GCollisionCheck::ABOVE:
			std::cout << ("No collision: Object is above the other.") << std::endl;
			break;
		case GW::MATH::GCollision::GCollisionCheck::BELOW:
			std::cout << ("No collision: Object is below the other.") << std::endl;
			break;
		case GW::MATH::GCollision::GCollisionCheck::NO_COLLISION:
			std::cout << ("No collision detected.") << std::endl;
			break;
		case GW::MATH::GCollision::GCollisionCheck::COLLISION:
			std::cout << ("Collision detected!") << std::endl;
			break;
		default:
			std::cout << ("Unknown collision result.") << std::endl;
			break;
		}
	}

	void ScaleTransform(GW::MATH::GVECTORF& inputVector, GW::MATH::GVECTORF& scalingVector, GW::MATH::GVECTORF& outputVector)
	{
		outputVector.x = inputVector.x * scalingVector.x;
		outputVector.y = inputVector.y * scalingVector.y;
		outputVector.z = inputVector.z * scalingVector.z;
	}

	void MoveColliderToWorldSpace(const GW::MATH::GMATRIXF& transform, GW::MATH::GOBBF& collider)
	{
		GW::MATH::GMatrix::VectorXMatrixF(transform, collider.center, collider.center);

		GW::MATH::GVECTORF scale;
		GW::MATH::GMatrix::GetScaleF(transform, scale);
		ScaleTransform(collider.extent, scale, collider.extent);

		GW::MATH::GQUATERNIONF transformRot;
		GW::MATH::GQuaternion::SetByMatrixF(transform, transformRot);
		GW::MATH::GQuaternion::MultiplyQuaternionF(collider.rotation, transformRot, collider.rotation);
	}

	void BounceOffWall(GW::MATH::GVECTORF EnemyLocation, GW::MATH::GVECTORF& EnemyVelocity, GW::MATH::GOBBF& ObstacleBox)
	{

		GW::MATH::GVECTORF Normal;
		GW::MATH::GCollision::ClosestPointToOBBF(ObstacleBox, EnemyLocation, Normal);
		GW::MATH::GVector::SubtractVectorF(EnemyLocation, Normal, Normal);
		Normal.y = 0.0f;
		Normal.w = 0.0f;
		GW::MATH::GVector::NormalizeF(Normal, Normal);

		float dot;
		GW::MATH::GVector::DotF(EnemyVelocity, Normal, dot);
		dot *= 2.0f;
		GW::MATH::GVector::ScaleF(Normal, dot, Normal);

		GW::MATH::GVector::SubtractVectorF(EnemyVelocity, Normal, EnemyVelocity);
		int i = 0;
	}

	void HandleCollision(GW::MATH::GCollision::GCollisionCheck collision,
		entt::registry& registry,
		entt::entity entityA,
		entt::entity entityB,
		const GW::MATH::GMATRIXF& transformA,
		const GW::MATH::GMATRIXF& transformB,
		GW::MATH::GOBBF& meshCollectionA,
		GW::MATH::GOBBF& meshCollectionB)

	{
		std::shared_ptr<const GameConfig> config = registry.ctx().get<UTIL::Config>().gameConfig;
		float invulnDuration = (*config).at("Player").at("invulnPeriod").as<float>();

		if (collision == GW::MATH::GCollision::GCollisionCheck::COLLISION)
		{
			if (registry.all_of<Bullet>(entityA) && registry.all_of<Obstacle>(entityB))
			{
				registry.emplace<Destroy>(entityA);

			}

			if (registry.all_of<Bullet>(entityB) && registry.all_of<Obstacle>(entityA))
			{
				registry.emplace<Destroy>(entityB);

			}

			if (registry.all_of<Enemy>(entityA) && registry.all_of<Obstacle>(entityB))
			{
				GW::MATH::GVECTORF& currentVelocity = registry.get<Velocity>(entityA).direction;
				BounceOffWall(transformA.row4, currentVelocity, meshCollectionB);
			}
			if (registry.all_of<Enemy>(entityB) && registry.all_of<Obstacle>(entityA))
			{
				GW::MATH::GVECTORF& currentVelocity = registry.get<Velocity>(entityB).direction;
				BounceOffWall(transformB.row4, currentVelocity, meshCollectionA);
			}

			if (registry.all_of<Bullet>(entityA) && registry.all_of<Enemy>(entityB))
			{
				registry.emplace<Destroy>(entityA);

				auto& eview = registry.view<Enemy>();
				auto& bview = registry.view<Bullet>();
				std::cout << "Size Enemy" << eview.size() << std::endl;
				std::cout << "Size Bullet" << bview.size() << std::endl;


				int& health = registry.get<Health>(entityB).health;
				health--;
				std::cout << "Enemy Health now: " << health << std::endl;
			}

			if (registry.all_of<Bullet>(entityB) && registry.all_of<Enemy>(entityA))
			{
				registry.emplace<Destroy>(entityB);

				auto& eview = registry.view<Enemy>();
				auto& bview = registry.view<Bullet>();
				std::cout << "Size Enemy" << eview.size() << std::endl;
				std::cout << "Size Bullet" << bview.size() << std::endl;


				int& health = registry.get<Health>(entityA).health;
				health--;
				std::cout << "Enemy Health now: " << health << std::endl;
			}

			if (registry.all_of<Player>(entityA) && registry.all_of<Enemy>(entityB))
			{
				if (!registry.try_get<Invuln>(entityA))
				{
					int& playerHealth = registry.get<Health>(entityA).health;
					playerHealth--;

					std::cout << "Player Health now: " << playerHealth << std::endl;

					registry.emplace<Invuln>(entityA, invulnDuration);
				}
			}

			if (registry.all_of<Player>(entityB) && registry.all_of<Enemy>(entityA))
			{
				if (!registry.try_get<Invuln>(entityB))
				{
					int& playerHealth = registry.get<Health>(entityB).health;
					playerHealth--;

					std::cout << "Player Health now: " << playerHealth << std::endl;

					float invulnDuration = 1.0f;
					registry.emplace<Invuln>(entityB, invulnDuration);
				}
			}
		}
	}

	void GameOverCheck(entt::registry& registry)
	{
		auto gameManagerView = registry.view<GameManager, GameOver>();
		if (gameManagerView.begin() != gameManagerView.end());
		
		auto playerView = registry.view<Player, Health>();
		bool allPlayersDead = true;

		for (auto player : playerView)
		{
			auto& health = playerView.get<Health>(player);
			if (health.health > 0)
			{
				allPlayersDead = false;
				break;
			}
		}

		if (allPlayersDead)
		{
			std::cout << "You lose Game Over" << std::endl;

			auto gameManagerEntity = registry.view<GameManager>().front();
			registry.emplace<GameOver>(gameManagerEntity);
			return;
		}

		auto enemyView = registry.view<Enemy>();
		if (enemyView.empty())
		{
			std::cout << "You win NICE" << std::endl;

			auto gameManagerEntity = registry.view<GameManager>().front();
			registry.emplace<GameOver>(gameManagerEntity);
		}
	}

	void PrintTransformMatrix(const Transform& t)
	{
		std::cout << "New Transform: " << std::endl;
		std::cout << "Row 1: [" << t.matrix.row1.x << ", " << t.matrix.row1.y
			<< ", " << t.matrix.row1.z << ", " << t.matrix.row1.w << "]" << std::endl;
		std::cout << "Row 2: [" << t.matrix.row2.x << ", " << t.matrix.row2.y
			<< ", " << t.matrix.row2.z << ", " << t.matrix.row2.w << "]" << std::endl;
		std::cout << "Row 3: [" << t.matrix.row3.x << ", " << t.matrix.row3.y
			<< ", " << t.matrix.row3.z << ", " << t.matrix.row3.w << "]" << std::endl;
		std::cout << "Row 4: [" << t.matrix.row4.x << ", " << t.matrix.row4.y
			<< ", " << t.matrix.row4.z << ", " << t.matrix.row4.w << "]" << std::endl;
	}

	void ShatterHandler(entt::registry& registry)
	{
		auto& view = registry.view<Enemy>();
		for (auto entity : view)
		{
			auto& health = registry.get<Health>(entity);
			auto& transform = registry.get<Transform>(entity);
			if (health.health <= 0)
			{
				registry.emplace<Destroy>(entity);

				if (registry.try_get<Shatter>(entity))
				{
					auto& shatters = registry.get<Shatter>(entity);
					shatters.initialShatterCount--;


					if (shatters.initialShatterCount > 0)
					{
						std::shared_ptr<const GameConfig> config = registry.ctx().get<UTIL::Config>().gameConfig;

						std::cout << "Enemy entity " << (int)entity << std::endl;
						for (int i = 0; i < shatters.shatterAmount; i++)
						{
							entt::entity newEnemy = registry.create();
							registry.emplace_or_replace< Transform>(entity, transform);

							auto newTransform = transform;
							float& scaleF = shatters.shatterScale;
							GW::MATH::GVECTORF scale{ scaleF,scaleF,scaleF, 0.0f};
							GW::MATH::GMatrix::ScaleLocalF(newTransform.matrix, scale, newTransform.matrix);

							std::cout << "firstTransform" << std::endl;

							registry.emplace<Transform>(newEnemy, newTransform);
							PrintTransformMatrix(newTransform);

							registry.emplace<Health>(newEnemy, health.maxHealth, health.maxHealth);
							registry.emplace<Collidable>(newEnemy);
							registry.emplace<Enemy>(newEnemy);
							if (shatters.initialShatterCount > 0)
								registry.emplace<Shatter>(newEnemy, shatters.initialShatterCount, shatters.shatterAmount, shatters.shatterScale);
							std::cout << shatters.initialShatterCount << std::endl;

							GW::MATH::GVECTORF velocity = UTIL::GetRandomVelocityVector();
							float enemySpeed = config->at("Enemy1").at("speed").as<float>();

							GW::MATH::GVector::NormalizeF(velocity, velocity);
							GW::MATH::GVector::ScaleF(velocity, enemySpeed, velocity);

							registry.emplace<Velocity>(newEnemy, velocity);
							auto& modelManager = registry.ctx().get<DRAW::ModelManager>();
							std::string enemyModelName = config->at("Enemy1").at("model").as<std::string>();

							std::cout << "secondTransform" << std::endl;
							PrintTransformMatrix(registry.get<Transform>(newEnemy));
							UTIL::SetupGameEntity(registry, modelManager, enemyModelName, newEnemy);

							std::cout << "thirdTransform" << std::endl;
							PrintTransformMatrix(registry.get<Transform>(newEnemy));
						}
					}
				}
				std::cout << "Enemies Remaining: " << view.size() << std::endl;
			}


		}
	}

	void UpdateCollisions(entt::registry& registry)
	{
		auto view = registry.view<GAME::Collidable, GAME::Transform, DRAW::MeshCollection>();

		for (auto entityA = view.begin(); entityA != view.end(); entityA++)
		{

			auto meshCollectionA = registry.get<DRAW::MeshCollection>(*entityA).obb;

			auto& transformA = registry.get<Transform>(*entityA).matrix;

			MoveColliderToWorldSpace(transformA, meshCollectionA);

			auto entityB = entityA;

			for (entityB++; entityB != view.end(); entityB++)
			{
				auto meshCollectionB = registry.get<DRAW::MeshCollection>(*entityB).obb;

				auto& transformB = registry.get<Transform>(*entityB).matrix;
				MoveColliderToWorldSpace(transformB, meshCollectionB);

				GW::MATH::GCollision::GCollisionCheck collision;
				GW::MATH::GCollision::TestOBBToOBBF(meshCollectionA, meshCollectionB, collision);

				HandleCollision(collision, registry, *entityA, *entityB, transformA, transformB, meshCollectionA, meshCollectionB);
			}
		}
	}

	void Cleanup(entt::registry& registry)
	{
		auto view = registry.view<GAME::Destroy>();
		for (auto entity : view)
		{
			registry.destroy(entity);
		}
	}

	void UpdateGameManager(entt::registry& registry, entt::entity entity) {

		if (!registry.view<GameOver>().empty()) {
			return;
		}

		GameOverCheck(registry);

		auto velocityView = registry.view<GAME::Transform, GAME::Velocity>();
		for (auto& entity : velocityView)
		{
			auto& transform = velocityView.get<Transform>(entity);
			auto& velocity = velocityView.get<Velocity>(entity);

			auto& deltaTime = registry.ctx().emplace<UTIL::DeltaTime>().dtSec;
			GW::MATH::GVECTORF deltaVelocity;
			GW::MATH::GVector::ScaleF(velocity.direction, deltaTime, deltaVelocity);

			GW::MATH::GMatrix::TranslateLocalF(transform.matrix, deltaVelocity, transform.matrix);
		}

		auto view = registry.view<GAME::Transform, DRAW::MeshCollection>();

		for (auto entity : view)
		{
			auto& transform = view.get<GAME::Transform>(entity);
			auto& meshCollection = view.get<DRAW::MeshCollection>(entity);

			for (auto meshEntity : meshCollection.meshEntities)
			{
				if (registry.all_of<DRAW::GPUInstance>(meshEntity))
				{
					auto& gpuInstance = registry.get<DRAW::GPUInstance>(meshEntity);

					gpuInstance.transform = transform.matrix;
				}
			}
		}

		auto playerView = registry.view<GAME::Player, GAME::Transform>();
		for (auto player : playerView)
		{
			registry.patch<GAME::Player>(player);
		}

		UpdateCollisions(registry);
		ShatterHandler(registry);
		Cleanup(registry);
	}

	CONNECT_COMPONENT_LOGIC() {
		registry.on_update<GameManager>().connect<&UpdateGameManager>();
	}

}