#include "GameComponents.h"
#include "../DRAW/DrawComponents.h"
#include "../UTIL/Utilities.h"
#include "..\CCL.h"	

namespace GAME {

	void SpawnBullet(entt::registry& registry, DRAW::ModelManager modelManager, const std::string modelName, entt::entity gameEntity, Transform transform)
	{
		auto& meshMap = modelManager.modelMap.find(modelName);

		if (meshMap == modelManager.modelMap.end())
		{
			std::cout << "Error: No mesh Map collection found for " << &modelName << "." << std::endl;
		}

		auto meshCollection = meshMap->second.meshEntities;

		auto& me = modelManager.modelMap[modelName];

		auto& gameMeshCollection = registry.emplace<DRAW::MeshCollection>(gameEntity);

		for (auto meshEntity : meshCollection)
		{
			const auto originalGeoData = registry.get<DRAW::GeometryData>(meshEntity);
			auto originalGpuInstance = registry.get<DRAW::GPUInstance>(meshEntity);
			originalGpuInstance.transform = transform.matrix;

			entt::entity newMeshEntity = registry.create();

			registry.emplace<DRAW::GeometryData>(newMeshEntity, originalGeoData);
			registry.emplace<DRAW::GPUInstance>(newMeshEntity, originalGpuInstance);

			gameMeshCollection.meshEntities.push_back(newMeshEntity);
			gameMeshCollection.obb = me.obb;

		}

	}

	void UpdatePlayer(entt::registry& registry, entt::entity entity)
	{
		auto& transform = registry.get<GAME::Transform>(entity);
		auto& config = registry.ctx().get<UTIL::Config>().gameConfig;
		auto& input = registry.ctx().get<UTIL::Input>();
		auto& deltaTime = registry.ctx().get<UTIL::DeltaTime>().dtSec;

		float speed;
		float firerate;
		try {
			speed = config->at("Player").at("speed").as<float>();
			firerate = config->at("Player").at("firerate").as<float>();
		}
		catch (...) {
			speed = 8.0f;
			firerate = 0.5f;
		}

		if (registry.try_get<GAME::Invuln>(entity))
		{
			auto& invuln = registry.get<GAME::Invuln>(entity);
			invuln.cooldown -= deltaTime;

			if (invuln.cooldown <= 0)
			{
				registry.remove<GAME::Invuln>(entity);
			}
			else if (invuln.cooldown < 0)
			{
				invuln.cooldown = 0;
			}
		}

		GW::MATH::GVECTORF moveVector = { 0.0f, 0.0f, 0.0f };

		float w, a, s, d;
		input.immediateInput.GetState(G_KEY_W, w);
		input.immediateInput.GetState(G_KEY_S, s);
		input.immediateInput.GetState(G_KEY_A, a);
		input.immediateInput.GetState(G_KEY_D, d);

		if (w) moveVector.z += 1.0f;
		if (s) moveVector.z -= 1.0f;
		if (a) moveVector.x -= 1.0f;
		if (d) moveVector.x += 1.0f;


		GW::MATH::GVector::NormalizeF(moveVector, moveVector);

		GW::MATH::GVECTORF scaledMove;
		GW::MATH::GVector::ScaleF(moveVector, speed * deltaTime, scaledMove);

		GW::MATH::GMatrix::TranslateLocalF(transform.matrix, scaledMove, transform.matrix);

		auto& newTransform = registry.get<Transform>(entity);
		newTransform.matrix = transform.matrix;
		registry.emplace_or_replace<Transform>(entity, newTransform);

		if (registry.all_of<Firing>(entity)) {
			auto& firing = registry.get<Firing>(entity);
			firing.cooldown -= deltaTime;
			if (firing.cooldown <= 0.0f) {
				registry.remove<Firing>(entity);
			}
		}



		else {
			float left, right, up, down;
			input.immediateInput.GetState(G_KEY_LEFT, left);
			input.immediateInput.GetState(G_KEY_RIGHT, right);
			input.immediateInput.GetState(G_KEY_UP, up);
			input.immediateInput.GetState(G_KEY_DOWN, down);

			GW::MATH::GVECTORF fireDirection = { 0.0f, 0.0f, 0.0f };

			if (up)    fireDirection.z += 1.0f;
			if (down)  fireDirection.z -= 1.0f;
			if (left)  fireDirection.x -= 1.0f;
			if (right) fireDirection.x += 1.0f;

			if (left || right || down || up)
			{
				GW::MATH::GVector::NormalizeF(fireDirection, fireDirection);
				float bulletSpeed = config->at("Bullet").at("speed").as<float>();  
				GW::MATH::GVector::ScaleF(fireDirection, bulletSpeed, fireDirection);

				//SpawnBullet(registry, config, transform, fireDirection);
				auto modelManager = registry.ctx().get<DRAW::ModelManager>();
				auto bulletName = config->at("Bullet").at("model").as<std::string>();

				auto bullet = registry.create();
				registry.emplace<GAME::Transform>(bullet, transform);
				registry.emplace<GAME::Bullet>(bullet);
				registry.emplace<GAME::Collidable>(bullet);
				registry.emplace<GAME::Velocity>(bullet, fireDirection);
				SpawnBullet(registry, modelManager, bulletName, bullet, transform);
				registry.emplace<Firing>(entity, Firing{ firerate });
			}
		}

	}

	CONNECT_COMPONENT_LOGIC() {

		registry.on_update<Player>().connect<&UpdatePlayer>();

	}

}