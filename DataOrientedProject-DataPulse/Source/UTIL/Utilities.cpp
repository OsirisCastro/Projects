#include "Utilities.h"
#include "../CCL.h"
namespace UTIL
{
	GW::MATH::GVECTORF GetRandomVelocityVector()
	{
		GW::MATH::GVECTORF vel = {float((rand() % 20) - 10), 0.0f, float((rand() % 20) - 10)};
		if (vel.x <= 0.0f && vel.x > -1.0f)
			vel.x = -1.0f;
		else if (vel.x >= 0.0f && vel.x < 1.0f)
			vel.x = 1.0f;

		if (vel.z <= 0.0f && vel.z > -1.0f)
			vel.z = -1.0f;
		else if (vel.z >= 0.0f && vel.z < 1.0f)
			vel.z = 1.0f;

		GW::MATH::GVector::NormalizeF(vel, vel);

		return vel;
	}

	void SetupGameEntity(entt::registry& registry, DRAW::ModelManager modelManager, const std::string& modelName, entt::entity gameEntity)
	{

		auto& mesh = modelManager.modelMap.find(modelName);

		if (mesh == modelManager.modelMap.end())
		{
			std::cout << "Error: No mesh collection found for " << &modelName << "." << std::endl;
		}

		auto& meshCollection = mesh->second.meshEntities;
		auto& me = modelManager.modelMap[modelName];

		auto& gameMeshCollection = registry.emplace<DRAW::MeshCollection>(gameEntity);

		for (auto meshEntity : meshCollection)
		{
			const auto originalGeoData = registry.get<DRAW::GeometryData>(meshEntity);
			const auto originalGpuInstance = registry.get<DRAW::GPUInstance>(meshEntity);

			entt::entity newMeshEntity = registry.create();

			registry.emplace<DRAW::GeometryData>(newMeshEntity, originalGeoData);
			registry.emplace<DRAW::GPUInstance>(newMeshEntity, originalGpuInstance);

			gameMeshCollection.meshEntities.push_back(newMeshEntity);
			gameMeshCollection.obb = me.obb;

		}

		if (!meshCollection.empty())
		{
			auto& gpuInstance = registry.get<DRAW::GPUInstance>(meshCollection.front());

			if (!registry.all_of<GAME::Transform>(gameEntity))
			{
				registry.emplace<GAME::Transform>(gameEntity);
				auto& transform = registry.get<GAME::Transform>(gameEntity);
				transform.matrix = gpuInstance.transform;
			}
			else
			{
				auto& transform = registry.get<GAME::Transform>(gameEntity);
				gpuInstance.transform = transform.matrix;
			}
		}
	}

} // namespace UTIL