// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "Components/RigidBodyComponent.hpp"
#include "Engine.hpp"
#include "SpritesData.hpp"
#include "Texture.hpp"

using namespace GLVM;
namespace cm = GLVM::ecs::components;

// Forward declarations
struct GameResources {
	cm::MeshHandle hyperCubeHandle;
	ecs::TextureHandle glvmTextureHandle;
};

// Function declarations
GameResources LoadGameAssets(core::Engine* engine);
Entity CreatePlayerEntity(ecs::EntityManager* entityManager, ecs::ComponentManager* componentManager);
Entity CreateGroundPlane(ecs::EntityManager* entityManager, ecs::ComponentManager* componentManager, 
                        const GameResources& resources);
Entity CreatePointLight(ecs::EntityManager* entityManager, ecs::ComponentManager* componentManager, 
                       const GameResources& resources);

GameResources LoadGameAssets(core::Engine* engine)
{
	GameResources resources;
	resources.hyperCubeHandle = engine->LoadMeshFromFile_GLTF("../gltf/hyper_cube.gltf");
	resources.glvmTextureHandle = engine->LoadTextureFromAddress(128, 128, glvm_dat_len, glvm_dat);
	return resources;
}

Entity CreatePlayerEntity(ecs::EntityManager* entityManager, ecs::ComponentManager* componentManager)
{
	Entity player = entityManager->CreateEntity();
	componentManager->CreateComponent<cm::mesh, cm::controller, cm::collider, cm::animation, cm::beholder,
		cm::transform, cm::rigidBody, cm::event>(player);
		
	// Configure player transform
	*componentManager->GetComponent<cm::transform>(player) = { 
		.tPosition = { 2.7f, 10.0f, 3.0f }, 
		.fScale = 1.0f 
	};
	
	// Configure player physics
	*componentManager->GetComponent<cm::rigidBody>(player) = { 
		.fMass_ = 6.0f 
	};
	
	// Configure player camera
	*componentManager->GetComponent<cm::beholder>(player) = { 
		.forward = { 0.0f, 0.0f, -1.0f },
		.up = { 0.0f, 1.0f, 0.0f } 
	};
	
	return player;
}

Entity CreateGroundPlane(ecs::EntityManager* entityManager, ecs::ComponentManager* componentManager, 
                        const GameResources& resources)
{
	Entity ground = entityManager->CreateEntity();
	componentManager->CreateComponent<cm::material, cm::mesh, cm::transform, cm::collider>(ground);
	
	// Configure ground transform
	*componentManager->GetComponent<cm::transform>(ground) = { 
		.tPosition = { 0.0f, -20.5f, 0.0f }, 
		.yaw = 10.0f, 
		.pitch = 0.0f, 
		.fScale = 20.2f, 
		.gltf = true 
	};
	
	// Set ground mesh
	componentManager->GetComponent<cm::mesh>(ground)->handle = resources.hyperCubeHandle;
	
	// Configure ground material
	*componentManager->GetComponent<cm::material>(ground) = { 
		.diffuseTextureID_ = resources.glvmTextureHandle, 
		.specularTextureID_ = resources.glvmTextureHandle, 
		.ambient = { 0.05f, 0.05f, 0.0f },
		.shininess = 128.0f * 0.078125f 
	};
	
	return ground;
}

Entity CreatePointLight(ecs::EntityManager* entityManager, ecs::ComponentManager* componentManager, 
                       const GameResources& resources)
{
	Entity light = entityManager->CreateEntity();
	componentManager->CreateComponent<cm::mesh, cm::material, cm::pointLight, cm::transform>(light);
	
	// Configure light properties
	*componentManager->GetComponent<cm::pointLight>(light) = { 
		.position = { 0.0f, 15.0f, 2.0f },
		.ambient = { 0.1f, 0.1f, 0.1f }, 
		.diffuse = { 0.8f, 0.8f, 0.8f }, 
		.specular = { 2.0f, 2.0f, 2.0f },
		.constant = 1.0f, 
		.linear = 0.09f, 
		.quadratic = 0.032f 
	};
	
	// Configure light transform
	*componentManager->GetComponent<cm::transform>(light) = { 
		.tPosition = { 0.0f, 15.0f, 2.0f }, 
		.fScale = 0.2f 
	};
	
	// Set light mesh
	componentManager->GetComponent<cm::mesh>(light)->handle = resources.hyperCubeHandle;
	
	// Configure light material
	*componentManager->GetComponent<cm::material>(light) = { 
		.diffuseTextureID_ = resources.glvmTextureHandle, 
		.specularTextureID_ = resources.glvmTextureHandle 
	};
	
	return light;
}

int main()
{
	// Initialize engine systems
	ecs::EntityManager* entityManager = ecs::EntityManager::GetInstance();
	ecs::ComponentManager* componentManager = ecs::ComponentManager::GetInstance();
	core::Engine* engine = core::Engine::GetInstance();
	
	// Load game assets
	GameResources resources = LoadGameAssets(engine);
	
	// Create game entities
	Entity player = CreatePlayerEntity(entityManager, componentManager);
	Entity ground = CreateGroundPlane(entityManager, componentManager, resources);
	Entity pointLight = CreatePointLight(entityManager, componentManager, resources);
	
	// Start game loop and cleanup
	engine->GameLoop(core::OPENGL_RENDERER);
	engine->GameKill();

	return 0;
}
