// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "Components/RigidBodyComponent.hpp"
#include "Engine.hpp"
#include "SpritesData.hpp"
#include "Texture.hpp"
#include "TimerCreator.hpp"
#include <vector>
#include <random>
#include <thread>
#include <atomic>
#include <chrono>

using namespace GLVM;
namespace cm = GLVM::ecs::components;

// Global variables for cube spawning system
static std::vector<Entity> fallingCubes;
static GLVM::Time::IChrono* gameTimer = nullptr;
static double lastSpawnTime = 0.0;
static const double SPAWN_INTERVAL = 0.8; // Spawn cube every 0.8 seconds - increased frequency
static const float GROUND_Y_LEVEL = -20.0f; // Y level below which cubes are destroyed
static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<float> positionDist(-5.0f, 5.0f);
static std::atomic<bool> gameRunning(true);

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
Entity CreateFallingCube(ecs::EntityManager* entityManager, ecs::ComponentManager* componentManager, 
                        const GameResources& resources, float x, float z, float playerY);
void UpdateFallingCubes(ecs::EntityManager* entityManager, ecs::ComponentManager* componentManager);
void SpawnCubeIfNeeded(ecs::EntityManager* entityManager, ecs::ComponentManager* componentManager, 
                      const GameResources& resources, Entity player);
void CubeManagementLoop(ecs::EntityManager* entityManager, ecs::ComponentManager* componentManager, 
                       const GameResources& resources, Entity player);

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
	};	// Configure player physics
	*componentManager->GetComponent<cm::rigidBody>(player) = { 
		.gravityTime = 0.0f,
		.fMass_ = 12.0f,  // Increased mass for faster falling
		.bGravity_ = true,
		.jump = vec3(0.0f, 18.0f, 0.0f),  // Increased jump force to compensate for higher mass
		.jumpAccumulator = 0.0f
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

Entity CreateFallingCube(ecs::EntityManager* entityManager, ecs::ComponentManager* componentManager, 
                        const GameResources& resources, float x, float z, float playerY)
{
	Entity cube = entityManager->CreateEntity();
	componentManager->CreateComponent<cm::mesh, cm::material, cm::transform, cm::rigidBody, cm::collider>(cube);
		// Configure cube transform - spawn 15 units above current player position
	*componentManager->GetComponent<cm::transform>(cube) = { 
		.tPosition = vec3(x, playerY + 5.0f, z), 
		.fScale = 1.0f,
		.gltf = true 
	};
	
	// Configure cube physics - enable gravity with reduced speed
	*componentManager->GetComponent<cm::rigidBody>(cube) = { 
		.fMass_ = 0.1f,  // Increased mass to slow down falling
		.bGravity_ = true
	};
	
	// Set cube mesh
	componentManager->GetComponent<cm::mesh>(cube)->handle = resources.hyperCubeHandle;
	
	// Configure cube material with different color
	*componentManager->GetComponent<cm::material>(cube) = { 
		.diffuseTextureID_ = resources.glvmTextureHandle, 
		.specularTextureID_ = resources.glvmTextureHandle, 
		.ambient = { 0.2f, 0.1f, 0.1f },
		.shininess = 64.0f 
	};
	
	return cube;
}

void UpdateFallingCubes(ecs::EntityManager* entityManager, ecs::ComponentManager* componentManager)
{
	// Check all falling cubes and remove those that fell below ground level
	auto it = fallingCubes.begin();
	while (it != fallingCubes.end()) {
		Entity cube = *it;
		cm::transform* transform = componentManager->GetComponent<cm::transform>(cube);
		
		if (transform && transform->tPosition[1] < GROUND_Y_LEVEL) {
			// Cube fell below ground, destroy it
			entityManager->RemoveEntity(cube, componentManager);
			it = fallingCubes.erase(it);
		} else {
			++it;
		}
	}
}

void SpawnCubeIfNeeded(ecs::EntityManager* entityManager, ecs::ComponentManager* componentManager, 
                      const GameResources& resources, Entity player)
{
	if (!gameTimer) return;
	
	double currentTime = gameTimer->GetElapsed();
		// Check if it's time to spawn a new cube
	if (currentTime - lastSpawnTime >= SPAWN_INTERVAL) {
		// Get current player position
		cm::transform* playerTransform = componentManager->GetComponent<cm::transform>(player);
		float playerY = playerTransform ? playerTransform->tPosition[1] : 10.0f;
		float playerX = playerTransform ? playerTransform->tPosition[0] : 0.0f;
		float playerZ = playerTransform ? playerTransform->tPosition[2] : 0.0f;
		
		// Generate random position within 10 units radius from player
		float randomX = playerX + positionDist(gen);
		float randomZ = playerZ + positionDist(gen);
		
		// Create new falling cube relative to player position
		Entity newCube = CreateFallingCube(entityManager, componentManager, resources, randomX, randomZ, playerY);
		fallingCubes.push_back(newCube);
		
		lastSpawnTime = currentTime;
	}
}

void CubeManagementLoop(ecs::EntityManager* entityManager, ecs::ComponentManager* componentManager, 
                       const GameResources& resources, Entity player)
{
	while (gameRunning.load()) {
		// Spawn new cubes if needed
		SpawnCubeIfNeeded(entityManager, componentManager, resources, player);
		
		// Update and clean up falling cubes
		UpdateFallingCubes(entityManager, componentManager);
		
		// Sleep for a short time to avoid consuming too much CPU
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

int main()
{
	// Initialize engine systems
	ecs::EntityManager* entityManager = ecs::EntityManager::GetInstance();
	ecs::ComponentManager* componentManager = ecs::ComponentManager::GetInstance();
	core::Engine* engine = core::Engine::GetInstance();
	
	// Initialize timer for cube spawning
	GLVM::Time::CTimerCreator timerCreator;
	gameTimer = timerCreator.Create();
	gameTimer->InitFrequency();
	gameTimer->Reset();
	
	// Load game assets
	GameResources resources = LoadGameAssets(engine);
	
	// Create game entities
	Entity player = CreatePlayerEntity(entityManager, componentManager);
	Entity ground = CreateGroundPlane(entityManager, componentManager, resources);
	Entity pointLight = CreatePointLight(entityManager, componentManager, resources);
		// Start cube management thread
	std::thread cubeThread(CubeManagementLoop, entityManager, componentManager, std::ref(resources), player);
	
	// Start game loop and cleanup
	engine->GameLoop(core::OPENGL_RENDERER);
	
	// Stop cube management thread
	gameRunning.store(false);
	if (cubeThread.joinable()) {
		cubeThread.join();
	}
	
	// Cleanup timer
	if (gameTimer) {
		delete gameTimer;
		gameTimer = nullptr;
	}
	
	engine->GameKill();

	return 0;
}
