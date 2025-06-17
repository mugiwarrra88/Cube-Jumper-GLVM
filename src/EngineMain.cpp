// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "Components/RigidBodyComponent.hpp"
#include "Engine.hpp"
#include "SpritesData.hpp"
#include "Texture.hpp"
#include "TimerCreator.hpp"
#include "ProceduralMusicSystem.hpp"  // Add procedural music system
#include "ISoundEngine.hpp"          // Add sound engine interface
#include <vector>
#include <random>
#include <thread>
#include <atomic>
#include <chrono>
#include <memory>                    // For std::make_unique

using namespace GLVM;
namespace cm = GLVM::ecs::components;

// Global variables for cube spawning system
static std::vector<Entity> fallingCubes;
static std::vector<vec3> cubePositions; // Track cube positions for distance checking
static GLVM::Time::IChrono* gameTimer = nullptr;
static double lastSpawnTime = 0.0;
static const double SPAWN_INTERVAL = 1.5; 
static const float GROUND_Y_LEVEL = -20.0f; // Y level below which cubes are destroyed
static const float MIN_SPAWN_DISTANCE = 3.0f; // Minimum distance between cubes on X-Z plane
static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<float> positionDist(-7.0f, 7.0f);
static std::uniform_real_distribution<float> colorDist(0.2f, 1.0f); // For random colors
static std::atomic<bool> gameRunning(true);

// Global variable for procedural music system
static std::unique_ptr<GLVM::core::Sound::ProceduralMusicSystem> proceduralMusic;

// Forward declarations
struct GameResources {
	cm::MeshHandle hyperCubeHandle;
	ecs::TextureHandle glvmTextureHandle;
};

// Function declarations
GameResources LoadGameAssets(core::Engine* engine);
Entity CreatePlayerEntity(ecs::EntityManager* entityManager, ecs::ComponentManager* componentManager);
Entity CreateGroundPlane(ecs::EntityManager* entityManager, ecs::ComponentManager* componentManager, 
                        const GameResources& resources, const vec3& position = {0.0f, -20.5f, 0.0f});
Entity CreateFallingCube(ecs::EntityManager* entityManager, ecs::ComponentManager* componentManager, 
                        const GameResources& resources, float x, float z, float playerY);
vec3 GenerateRandomColor(); // Generate random color for cubes
bool IsPositionTooClose(float x, float z); // Check if position is too close to existing cubes

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

vec3 GenerateRandomColor()
{
	// Generate more vibrant random colors using predefined palette
	static const vec3 colors[] = {
		{1.0f, 0.0f, 0.0f},  // Red
		{0.0f, 1.0f, 0.0f},  // Green
		{0.0f, 0.0f, 1.0f},  // Blue
		{1.0f, 1.0f, 0.0f},  // Yellow
		{1.0f, 0.0f, 1.0f},  // Magenta
		{0.0f, 1.0f, 1.0f},  // Cyan
		{1.0f, 0.5f, 0.0f},  // Orange
		{0.5f, 0.0f, 1.0f},  // Purple
		{1.0f, 0.0f, 0.5f},  // Pink
		{0.0f, 1.0f, 0.5f}   // Light Green
	};
	
	static std::uniform_int_distribution<int> colorIndex(0, 9);
	return colors[colorIndex(gen)];
}

bool IsPositionTooClose(float x, float z)
{
	// Check if the new position is too close to any existing cube
	for (const vec3& pos : cubePositions) {
		float deltaX = x - pos[0];
		float deltaZ = z - pos[2];
		float distanceXZ = sqrt(deltaX * deltaX + deltaZ * deltaZ);
		
		if (distanceXZ < MIN_SPAWN_DISTANCE) {
			return true; // Too close to an existing cube
		}
	}
	return false; // Position is acceptable
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
		.gravityTime = 1.0f,
		.fMass_ = 25.0f,  // Increased mass for faster falling
		.bGravity_ = true,
		.jump = vec3(0.0f, 100.0f, 0.0f),  // Increased jump force to compensate for higher mass
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
                        const GameResources& resources, const vec3& position)
{
	Entity ground = entityManager->CreateEntity();
	componentManager->CreateComponent<cm::material, cm::mesh, cm::transform, cm::collider>(ground);
	
	// Configure ground transform with passed position
	*componentManager->GetComponent<cm::transform>(ground) = { 
		.tPosition = position, 
		.pitch = 90.0f,
		.fScale = 10.2f, 
		.gltf = true 
	};
	vec3 randomColor = GenerateRandomColor();

		// Set ground mesh
	componentManager->GetComponent<cm::mesh>(ground)->handle = resources.hyperCubeHandle;
		// Configure ground material - neutral gray color
	*componentManager->GetComponent<cm::material>(ground) = { 
		.diffuseTextureID_ = resources.glvmTextureHandle, 
		.specularTextureID_ = resources.glvmTextureHandle, 
		.ambient = randomColor,  // Gray color for ground
		.shininess = 1.0f 
	};
		return ground;
}

Entity CreateFallingCube(ecs::EntityManager* entityManager, ecs::ComponentManager* componentManager,
                        const GameResources& resources, float x, float z, float playerY)
{
	Entity cube = entityManager->CreateEntity();
	componentManager->CreateComponent<cm::mesh, cm::material, cm::transform, cm::rigidBody, cm::collider>(cube);
		// Configure cube transform - spawn 15 units above current player position
	*componentManager->GetComponent<cm::transform>(cube) = { 
		.tPosition = vec3(x, playerY + 1.0f, z), 
		.pitch = 90.0f,
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
		// Configure cube material with random color
	vec3 randomColor = GenerateRandomColor();
	*componentManager->GetComponent<cm::material>(cube) = { 
		.diffuseTextureID_ = resources.glvmTextureHandle, 
		.specularTextureID_ = resources.glvmTextureHandle, 
		.ambient = randomColor,  // Use random color instead of white
		.shininess = 1.0f 
	};
	
	return cube;
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
		
		// Try to find a valid spawn position (max 50 attempts to avoid infinite loop)
		float randomX, randomZ;
		int attempts = 0;
		const int maxAttempts = 50;
		
		do {
			randomX = playerX + positionDist(gen);
			randomZ = playerZ + positionDist(gen);
			attempts++;
		} while (IsPositionTooClose(randomX, randomZ) && attempts < maxAttempts);
		
		// If we found a valid position (or exhausted attempts), spawn the cube
		if (attempts < maxAttempts || cubePositions.empty()) {
			// Create new falling cube relative to player position
			Entity newCube = CreateFallingCube(entityManager, componentManager, resources, randomX, randomZ, playerY);
			fallingCubes.push_back(newCube);
			
			// Store the cube position for future distance checks
			cubePositions.push_back(vec3(randomX, playerY + 1.0f, randomZ));
			
			lastSpawnTime = currentTime;
		}
	}
}

void CubeManagementLoop(ecs::EntityManager* entityManager, ecs::ComponentManager* componentManager,
                       const GameResources& resources, Entity player)
{
	while (gameRunning.load()) {
		// Spawn new cubes if needed
		SpawnCubeIfNeeded(entityManager, componentManager, resources, player);
				// Check if player has fallen below Y = -50 and teleport back if so
		cm::transform* playerTransform = componentManager->GetComponent<cm::transform>(player);
		if (playerTransform && playerTransform->tPosition[1] < -50.0f) {
			// Teleport player back to starting position
			playerTransform->tPosition[0] = 2.7f;
			playerTransform->tPosition[1] = 10.0f;
			playerTransform->tPosition[2] = 3.0f;
			
			// Reset gravity time to prevent continued falling momentum
			cm::rigidBody* playerRigidBody = componentManager->GetComponent<cm::rigidBody>(player);
			if (playerRigidBody) {
				playerRigidBody->gravityTime = 1.0f;
				playerRigidBody->jumpAccumulator = 0.0f;
			}
		}
			
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
	GameResources resources = LoadGameAssets(engine);	// Create game entities
	Entity player = CreatePlayerEntity(entityManager, componentManager);
	CreateGroundPlane(entityManager, componentManager, resources, {0.0f, -20.0f, 0.0f});
	CreateGroundPlane(entityManager, componentManager, resources, {40.0f, 0.0f, 0.0f});
	CreateGroundPlane(entityManager, componentManager, resources, {40.0f*2, 20.0f, 0.0f});
	CreateGroundPlane(entityManager, componentManager, resources, {40.0f*3, 20.0f*2, 0.0f});
	
	CreateGroundPlane(entityManager, componentManager, resources, {40.0f*3, 20.0f*3, 40.0f});
	CreateGroundPlane(entityManager, componentManager, resources, {40.0f*3, 20.0f*4, 40.0f*2});
	CreateGroundPlane(entityManager, componentManager, resources, {40.0f*3, 20.0f*5, 40.0f*3});
	
	// Initialize procedural music system
	proceduralMusic = std::make_unique<GLVM::core::Sound::ProceduralMusicSystem>(engine->GetSoundEngine());
	
	// Configure music style - use pentatonic scale for a peaceful, ambient feel
	proceduralMusic->SetMusicStyle(GLVM::core::Sound::Scale::PENTATONIC, 70.0f); // Slow tempo
	
	// Start the procedural music
	proceduralMusic->Start();
	
	// Start cube management thread
	std::thread cubeThread(CubeManagementLoop, entityManager, componentManager, std::ref(resources), player);
	
	// Start game loop and cleanup
	engine->GameLoop(core::OPENGL_RENDERER);
	
	// Stop cube management thread
	gameRunning.store(false);
	if (cubeThread.joinable()) {
		cubeThread.join();
	}
	
	// Stop and cleanup procedural music system
	if (proceduralMusic) {
		proceduralMusic->Stop();
		proceduralMusic.reset();
	}
	
	// Cleanup timer
	if (gameTimer) {
		delete gameTimer;
		gameTimer = nullptr;
	}
	
	engine->GameKill();

	return 0;
}
