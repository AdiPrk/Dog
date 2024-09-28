#include <PCH/pch.h>

#ifndef DOG_SHIP

#include "InspectorWindow.h"
#include "EntitiesWindow.h"
#include "Dog/Scene/sceneManager.h"
#include "Dog/Scene/scene.h"
#include "Dog/Scene/Entity/entity.h"
#include "Dog/Scene/Entity/components.h"
#include "Input/input.h"
#include "Engine.h"
#include "Graphics/Vulkan/Models/ModelLibrary.h"
#include "Graphics/Vulkan/Models/Model.h"

namespace Dog {

	void DisplayComponents(Entity entity);

	template <typename T>
	void DisplayAddComponent(Entity entity, const std::string& name);

	void UpdateInspectorWindow() {
		ImGui::Begin("Inspector");

		// lock input if this window is focused
		// log is window focused
		Input::SetKeyInputLocked(ImGui::IsWindowFocused());

		static Entity lastSelectedEntity;
		Entity selectedEntity = GetSelectedEntity();

		bool entityChanged = lastSelectedEntity != selectedEntity;

		if (entityChanged) {
			lastSelectedEntity = selectedEntity;
		}

		if (!selectedEntity) {
			ImGui::Text("No entity selected.");
			ImGui::End(); // Inspector
			return;
		}

		if (entityChanged) {
			// ImGui::Text("Switching Entities...");
			// ImGui::End(); // Inspector
			// return; // don't display for one frame.
		}

		ImGui::PushID(selectedEntity);

		// Display components
		DisplayComponents(selectedEntity);

		// Add components button
		if (ImGui::Button("Add Component")) {
			ImGui::OpenPopup("AddComponentPopup");
		}

		if (ImGui::BeginPopup("AddComponentPopup")) {
			DisplayAddComponent<TagComponent>(selectedEntity, "Tag");
			DisplayAddComponent<TransformComponent>(selectedEntity, "Transform");
			DisplayAddComponent<ModelComponent>(selectedEntity, "Model");
			// DisplayAddComponent<SpriteComponent>(selectedEntity, "Sprite");
			// DisplayAddComponent<ShaderComponent>(selectedEntity, "Shader");
			// DisplayAddComponent<CameraComponent>(selectedEntity, "Camera");

			ImGui::EndPopup();
		}

		ImGui::PopID();

		ImGui::End(); // Inspector
	}

	template <typename T>
	void DisplayAddComponent(Entity entity, const std::string& name) {
		if (!entity.HasComponent<T>() && ImGui::MenuItem(name.c_str())) {
			entity.AddComponent<T>();
		}
	}

	void RenderTagComponent(TagComponent& tagComponent) {
		std::string& tag = tagComponent.Tag;

		char buffer[256];

		strncpy_s(buffer, tag.c_str(), sizeof(buffer));
		buffer[sizeof(buffer) - 1] = '\0';

		if (ImGui::InputText("Name##TagProp", buffer, sizeof(buffer))) {
			tag = std::string(buffer);
		}

		/*bool imguiDrawCursor = false;
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("The entity's name.");
			imguiDrawCursor = true;
		}

		ImGui::GetIO().MouseDrawCursor = imguiDrawCursor;*/
	}

	void RenderTransformComponent(TransformComponent& transform) {
		ImGui::SetNextItemOpen(false, ImGuiCond_FirstUseEver);

		if (ImGui::CollapsingHeader("Transform##header")) {
			ImGui::DragFloat3("Position##TransformProp", &transform.Translation.x, 0.1f);
			ImGui::DragFloat3("Rotation##TransformProp", &transform.Rotation.x, 0.1f);
			ImGui::DragFloat3("Scale##TransformProp", &transform.Scale.x, 0.1f);
		}
	}

	void RenderModelComponent(ModelComponent& model) {
		ImGui::SetNextItemOpen(false, ImGuiCond_FirstUseEver);

		if (ImGui::CollapsingHeader("Model##header")) {
			auto& ml = Engine::Get().GetModelLibrary();
			uint32_t modelCount = ml.GetModelCount();

			// begin combo
			std::vector<std::string> modelNames;
			std::vector<std::string> viewNames;
			modelNames.reserve(modelCount);
			viewNames.reserve(modelCount);

			for (uint32_t i = 0; i < modelCount; i++) {
				auto& str = modelNames.emplace_back(ml.GetModelByIndex(i)->GetPath());
				viewNames.emplace_back(str.substr(str.find_last_of('/') + 1));
			}

			static int currentModelIndex = -1;
			for (uint32_t i = 0; i < modelCount; i++) {
				if (model.ModelPath == modelNames[i]) {
					currentModelIndex = i;
					break;
				}
			}

			const char* modelPreviewValue = currentModelIndex == -1 ? "None" : viewNames[currentModelIndex].c_str();

			if (ImGui::BeginCombo("Model##ModelProp", modelPreviewValue)) {

				for (uint32_t i = 0; i < modelCount; i++) {
					const bool isSelected = currentModelIndex == i;

					if (ImGui::Selectable(viewNames[i].c_str(), isSelected)) {
						currentModelIndex = i;
						model.SetModel(modelNames[i]);
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
		}
	}

	void DisplayComponents(Entity entity) {
		// get all components from the entity
		entt::registry& registry = entity.GetScene()->GetRegistry();

		if (entity.HasComponent<TagComponent>())
			RenderTagComponent(entity.GetComponent<TagComponent>());

		if (entity.HasComponent<TransformComponent>())
			RenderTransformComponent(entity.GetComponent<TransformComponent>());

		if (entity.HasComponent<ModelComponent>())
			RenderModelComponent(entity.GetComponent<ModelComponent>());

		/*if (entity.HasComponent<SpriteComponent>())
			RenderSpriteComponent(entity.GetComponent<SpriteComponent>());

		if (entity.HasComponent<ShaderComponent>())
			RenderShaderComponent(entity.GetComponent<ShaderComponent>());

		if (entity.HasComponent<CameraComponent>())
			RenderCameraComponent(entity.GetComponent<CameraComponent>());*/
	}

}

#endif