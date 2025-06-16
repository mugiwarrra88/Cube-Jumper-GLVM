// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "Components/RigidBodyComponent.hpp"
#include "Engine.hpp"
#include "SpritesData.hpp"
#include "Texture.hpp"

int main()
{
	using namespace GLVM;
	namespace cm = GLVM::ecs::components;

	// Initialize engine systems
	ecs::EntityManager* EntityManager = ecs::EntityManager::GetInstance();
	ecs::ComponentManager* ComponentManager = ecs::ComponentManager::GetInstance();
	core::Engine* GLVM = core::Engine::GetInstance();
	
	// Load assets
	cm::MeshHandle hyperCubeHandle_GLTF = GLVM->LoadMeshFromFile_GLTF("../gltf/hyper_cube.gltf");
	ecs::TextureHandle glvmTextureHandle = GLVM->LoadTextureFromAddress(128, 128, glvm_dat_len, glvm_dat);
	// Create player entity
	Entity uiPlayer = EntityManager->CreateEntity();
	ComponentManager->CreateComponent<cm::mesh, cm::controller, cm::collider, cm::animation, cm::beholder,
		cm::transform, cm::rigidBody, cm::event>(uiPlayer);
		
	// Configure player components
	*ComponentManager->GetComponent<cm::transform>(uiPlayer) = { 
		.tPosition = { 2.7f, 10.0f, 3.0f }, 
		.fScale = 1.0f 
	};
	*ComponentManager->GetComponent<cm::rigidBody>(uiPlayer) = { 
		.fMass_ = 6.0f 
	};
	*ComponentManager->GetComponent<cm::beholder>(uiPlayer) = { 
		.forward = { 0.0f, 0.0f, -1.0f },
		.up = { 0.0f, 1.0f, 0.0f } 
	};
	// Create ground plane entity
	Entity plain0 = EntityManager->CreateEntity();
	ComponentManager->CreateComponent<cm::material, cm::mesh, cm::transform, cm::collider>(plain0);
	
	// Configure ground plane
	*ComponentManager->GetComponent<cm::transform>(plain0) = { 
		.tPosition = { 0.0f, -20.5f, 0.0f }, 
		.yaw = 10.0f, 
		.pitch = 0.0f, 
		.fScale = 20.2f, 
		.gltf = true 
	};
	ComponentManager->GetComponent<cm::mesh>(plain0)->handle = hyperCubeHandle_GLTF;
	
	cm::material* materialPlain0 = ComponentManager->GetComponent<cm::material>(plain0);
	*materialPlain0 = { 
		.diffuseTextureID_ = glvmTextureHandle, 
		.specularTextureID_ = glvmTextureHandle, 
		.ambient = { 0.05f, 0.05f, 0.0f },
		.shininess = 128.0f * 0.078125f 	};

	// Create point light entity
	Entity pointLight0 = EntityManager->CreateEntity();
	ComponentManager->CreateComponent<cm::mesh, cm::material, cm::pointLight, cm::transform>(pointLight0);
	
	// Configure point light
	*ComponentManager->GetComponent<cm::pointLight>(pointLight0) = { 
		.position = { 0.0f, 15.0f, 2.0f },
		.ambient = { 0.1f, 0.1f, 0.1f }, 
		.diffuse = { 0.8f, 0.8f, 0.8f }, 
		.specular = { 2.0f, 2.0f, 2.0f },
		.constant = 1.0f, 
		.linear = 0.09f, 
		.quadratic = 0.032f 
	};
	*ComponentManager->GetComponent<cm::transform>(pointLight0) = { 
		.tPosition = { 0.0f, 15.0f, 2.0f }, 
		.fScale = 0.2f 
	};
	ComponentManager->GetComponent<cm::mesh>(pointLight0)->handle = hyperCubeHandle_GLTF;
	
	cm::material* materialPointLight0 = ComponentManager->GetComponent<cm::material>(pointLight0);
	*materialPointLight0 = { 
		.diffuseTextureID_ = glvmTextureHandle, 
		.specularTextureID_ = glvmTextureHandle 
	};

	// Start game loop and cleanup

	GLVM->GameLoop(GLVM::core::OPENGL_RENDERER);
	GLVM->GameKill();

	return 0;
}
