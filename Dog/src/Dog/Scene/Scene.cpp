#include <PCH/pch.h>
#include "scene.h"
#include "Dog/Logger/logger.h"

#include "Entity/entity.h"
#include "Entity/components.h"
#include "Dog/engine.h"

#include "Dog/Graphics/Vulkan/Window/Window.h"

#define DOG_SCENE_LOGGING 0

namespace Dog {

	Scene::Scene(const std::string& name)
	{
		sceneName = name;

		/*FrameBufferSpecification fbSpec;
		fbSpec.width = 1280;
		fbSpec.height = 720;
		fbSpec.samples = 1;
		fbSpec.attachments = { FBAttachment::RGBA8, FBAttachment::Depth24Stencil8 };
		sceneFrameBuffer = std::make_shared<FrameBuffer>(fbSpec);*/

		width = Engine::Get().GetWindow().GetWidth();
		height = Engine::Get().GetWindow().GetHeight();

		// Shader::SetResolutionUBO(glm::vec2(fbSpec.width, fbSpec.height));

		//sceneOrthographicCamera = std::make_shared<SceneOrthographicCamera>((float)width / (float)height);
		//sceneOrthographicCamera->UpdateUniforms(); // Should be moved to the renderer (?) or somewhere else

		//scenePerspectiveCamera = std::make_shared<ScenePerspectiveCamera>();
		//scenePerspectiveCamera->UpdateUniforms(); // Should be moved to the renderer (?) or somewhere else

		// eventSceneFBResize = SUBSCRIBE_EVENT(Event::SceneResize, sceneFrameBuffer->OnSceneResize);
		// eventSceneOrthoCamResize = SUBSCRIBE_EVENT(Event::SceneResize, sceneOrthographicCamera->OnSceneResize);
		// eventScenePerspCamResize = SUBSCRIBE_EVENT(Event::SceneResize, scenePerspectiveCamera->OnSceneResize);
		// eventPlayButtonPressed = SUBSCRIBE_EVENT(Event::PlayButtonPressed, OnPlayButtonPressed);
		// eventStopButtonPressed = SUBSCRIBE_EVENT(Event::StopButtonPressed, OnStopButtonPressed);

#ifndef DOG_SHIP
		// Engine::GetEditor()->DoSceneResize();
#endif
	}

	Scene::~Scene()
	{
	}

	void Scene::OnPlayButtonPressed(const Event::PlayButtonPressed& event)
	{
		DOG_INFO("Play button pressed.");
	}

	void Scene::OnStopButtonPressed(const Event::StopButtonPressed& event)
	{
		DOG_INFO("Stop button pressed.");
	}

	void Scene::ClearEntities()
	{
		// reset registry
		registry.clear();
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity newEnt(this);

		newEnt.AddComponent<UUID>();
		TagComponent& tg = newEnt.AddComponent<TagComponent>();
		TransformComponent& tr = newEnt.AddComponent<TransformComponent>();
		ModelComponent& mc = newEnt.AddComponent<ModelComponent>("assets/models/quad.obj");

		tg.Tag = name;
		tr.Translation -= tr.Scale * 0.5f;

		return newEnt;
	}

	Entity Scene::CreateEntityFromUUID(const UUID& uuid, const std::string& name)
	{
		Entity newEnt(this);

		newEnt.AddComponent<UUID>(uuid);
		TagComponent& tg = newEnt.AddComponent<TagComponent>(name);
		TransformComponent& tr = newEnt.AddComponent<TransformComponent>();

		return newEnt;
	}

	Entity Scene::CreateEmptyEntity(const std::string& name)
	{
		Entity newEnt(this);

		newEnt.AddComponent<UUID>();
		TagComponent& tg = newEnt.AddComponent<TagComponent>(name);

		return newEnt;
	}

	void Scene::InternalInit()
	{
#if DOG_SCENE_LOGGING
		DOG_INFO("Scene {0} init.", sceneName);
#endif
	}

	void Scene::InternalUpdate(float dt)
	{
#if DOG_SCENE_LOGGING
		DOG_INFO("Scene {0} Update.", sceneName);
#endif

		/*if (isOrthographic) {
			sceneOrthographicCamera->OnUpdate(dt);
			sceneOrthographicCamera->UpdateUniforms();
		}
		else {
			scenePerspectiveCamera->OnUpdate(dt);
			scenePerspectiveCamera->UpdateUniforms();
		}*/
	}

	void Scene::InternalRender(float dt, bool renderEditor)
	{
#if DOG_SCENE_LOGGING
		DOG_INFO("Scene {0} Render.", sceneName);
#endif

		/*std::shared_ptr<Renderer2D>& renderer2D = Engine::Get().GetRenderer2D();

		if (renderEditor) {
			sceneFrameBuffer->Bind();
		}

		renderer2D->beginFrame();

		registry.view<TransformComponent, SpriteComponent>().each
		([&](const auto& entity, const TransformComponent& transform, const SpriteComponent& sprite) {

			auto texPtr = Assets::Get<Texture2D>(sprite.texturePath);
			if (!texPtr) {
				texPtr = Assets::Get<Texture2D>("error.png");
				if (!texPtr) return;
			}

			std::shared_ptr<Shader> shaderPtr;
			if (registry.all_of<ShaderComponent>(entity)) {
				ShaderComponent& shader = registry.get<ShaderComponent>(entity);
				shaderPtr = Assets::Get<Shader>(shader.shaderPath);

				if (!shaderPtr) {
					shaderPtr = Assets::Get<Shader>("error");
					if (!shaderPtr) {
						return;
					}
				}
			}
			else {
				shaderPtr = Assets::Get<Shader>("defaultsprite");
				if (!shaderPtr) {
					shaderPtr = Assets::Get<Shader>("error");
					if (!shaderPtr) {
						return;
					}
				}
			}

			renderer2D->DrawSprite(texPtr, shaderPtr, transform.GetTransform(), sprite.Color);
			});

		renderer2D->endFrame();

		if (renderEditor) {
			sceneFrameBuffer->Unbind();
		}*/

	}

	void Scene::InternalExit()
	{
#if DOG_SCENE_LOGGING
		DOG_INFO("Scene {0} exit.", sceneName);
#endif
	}

	glm::mat4 Scene::GetProjectionMatrix()
	{
		/*if (isOrthographic) {
			return sceneOrthographicCamera->GetProjectionMatrix();
		}
		else {
			return scenePerspectiveCamera->GetProjectionMatrix();
		}*/
		return glm::mat4(1.f);
	}

	glm::mat4 Scene::GetViewMatrix()
	{
		/*if (isOrthographic) {
			return sceneOrthographicCamera->GetViewMatrix();
		}
		else {
			return scenePerspectiveCamera->GetViewMatrix();
		}*/
		return glm::mat4(1.f);
	}

}
