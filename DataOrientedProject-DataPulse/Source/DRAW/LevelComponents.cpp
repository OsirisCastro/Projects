#include "DrawComponents.h"
#include "..\CCL.h"	
#include "../GAME/GameComponents.h"

namespace DRAW {

	void Update_VulkanVertexBuffer(entt::registry& registry, entt::entity entity);
	void Update_VulkanIndexBuffer(entt::registry& registry, entt::entity entity);


	void Construct_CPULevel(entt::registry& registry, entt::entity entity)
	{
		auto& cpuLevel = registry.get<CPULevel>(entity);
		GW::SYSTEM::GLog log;
		log.Create("levelLoading");
		log.EnableConsoleLogging(true);

		if (!cpuLevel.levelData.LoadLevel(cpuLevel.levelPath.c_str(), cpuLevel.modelPath.c_str(), log)) {
			log.Log("Failed to load level data!");
			return;
		}
		log.Log("Level data loaded successfully!");
	}

	void Construct_GPULevel(entt::registry& registry, entt::entity entity)
	{

		if (auto* cpuLevel = registry.try_get<CPULevel>(entity)) {

			GW::SYSTEM::GLog log;
			log.Create("levelLoading");
			log.EnableConsoleLogging(true);

			if (!cpuLevel->levelData.LoadLevel(cpuLevel->levelPath.c_str(), cpuLevel->modelPath.c_str(), log))
			{
				log.Log("CPULevel data is not loaded, skipping GPULevel construction");
				return;
			}

			auto& gpuLevel = registry.emplace_or_replace<GPULevel>(entity);

			registry.emplace<VulkanVertexBuffer>(entity);
			registry.emplace < std::vector < H2B::VERTEX>>(entity, cpuLevel->levelData.levelVertices);
			registry.patch<VulkanVertexBuffer>(entity);

			registry.emplace<VulkanIndexBuffer>(entity);
			registry.emplace < std::vector <unsigned int>>(entity, cpuLevel->levelData.levelIndices);
			registry.patch<VulkanIndexBuffer>(entity);

			entt::entity* ctxEntityPtr = registry.ctx().find<entt::entity>();

			if (!ctxEntityPtr) { 
				entt::entity newCtxEntity = registry.create();
				registry.ctx().emplace<entt::entity>(newCtxEntity);
				ctxEntityPtr = registry.ctx().find<entt::entity>(); 
			}

			entt::entity ctxEntity = *ctxEntityPtr;

			if (!registry.ctx().contains<DRAW::ModelManager>()) {
				registry.ctx().emplace<DRAW::ModelManager>();
			}

			auto& modelManager = registry.ctx().get<DRAW::ModelManager>();

			for (const auto& blenderObj : cpuLevel->levelData.blenderObjects)
			{
				const auto& model = cpuLevel->levelData.levelModels[blenderObj.modelIndex];

				DRAW::MeshCollection meshCollection;

				for (unsigned int meshIdx = 0; meshIdx < model.meshCount; ++meshIdx)
				{
					const auto& mesh = cpuLevel->levelData.levelMeshes[model.meshStart + meshIdx];

					entt::entity meshEntity = registry.create();

					registry.emplace<GeometryData>(meshEntity, model.indexStart + mesh.drawInfo.indexOffset,
					mesh.drawInfo.indexCount, model.vertexStart);

					registry.emplace<GPUInstance>(meshEntity, GPUInstance{
					cpuLevel->levelData.levelTransforms[blenderObj.transformIndex],
					cpuLevel->levelData.levelMaterials[model.materialStart + mesh.materialIndex].attrib
					});

					if (model.isDynamic)
					{
						registry.emplace<DRAW::DoNotRender>(meshEntity);
						meshCollection.meshEntities.push_back(meshEntity);
					}

				}

				meshCollection.obb = cpuLevel->levelData.levelColliders[model.colliderIndex];
				if (model.isDynamic)
				{
					modelManager.modelMap[blenderObj.blendername] = meshCollection;

				}
				if (model.isCollidable)
				{
					auto wall = registry.create();

					registry.emplace<DRAW::MeshCollection>(wall, meshCollection);
					registry.emplace<GAME::Collidable>(wall);
					registry.emplace<GAME::Transform>(wall, cpuLevel->levelData.levelTransforms[blenderObj.transformIndex]);
					registry.emplace< GW::MATH::GOBBF>(wall, cpuLevel->levelData.levelColliders[model.colliderIndex]);
					registry.emplace<GAME::Obstacle>(wall);

				}
			}
		}

		else
		{
			GW::SYSTEM::GLog log;
			log.Create("GPULevelSetup");
			log.Log("Sucessfully retrieve CPULevel.");
		}
	}

	void Destroy_MeshCollection(entt::registry& registry, entt::entity entity)
	{
		auto& meshCollection = registry.get<DRAW::MeshCollection>(entity);
		for (auto entity : meshCollection.meshEntities)
			registry.destroy(entity);
	}

	CONNECT_COMPONENT_LOGIC() {
		// register the Window component's logic
		registry.on_construct<CPULevel>().connect<Construct_CPULevel>();
		registry.on_construct<GPULevel>().connect<Construct_GPULevel>();
		registry.on_destroy<DRAW::MeshCollection>().connect<&Destroy_MeshCollection>();
	}
	
}