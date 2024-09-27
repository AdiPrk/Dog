#pragma once

namespace Dog {

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag)
		{
		}
	};

	struct TransformComponent
	{
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale)
			: Translation(translation)
			, Rotation(rotation)
			, Scale(scale)
		{
		}

		glm::mat4 GetTransform() const
		{
			// SRT - Translate * Rotate * Scale
			return glm::translate(glm::mat4(1.0f), Translation) *
				glm::toMat4(glm::quat(Rotation)) *
				glm::scale(glm::mat4(1.0f), Scale);
		}

		glm::mat4 mat4() const;
		glm::mat3 normalMatrix() const;
	};

	struct SpriteComponent
	{
		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		std::string texturePath;

		SpriteComponent();
		SpriteComponent(const SpriteComponent&) = default;
		SpriteComponent(const glm::vec4& color, const std::string& texturePath);
	};

	struct MaterialComponent
	{
		uint32_t albedoTexture = INVALID_TEXTURE_INDEX;
		uint32_t normalTexture = INVALID_TEXTURE_INDEX;
	};

	class Mesh;
	struct ModelComponent
	{
		uint32_t modelIndex = INVALID_MODEL_INDEX;
		std::string modelPath;

		ModelComponent() = default;
		ModelComponent(const ModelComponent&);
		ModelComponent(const std::string& modelPath);
	};

	/*class OrthographicCamera;
	class PerspectiveCamera;

	struct CameraComponent
	{
		void SetAsCurrentCamera();

		enum class CameraType
		{
			Orthographic = 0,
			Perspective
		};

		bool MainCamera = false;

		CameraType Projection = CameraType::Orthographic;

		float OrthographicSize = 1.0f;
		float OrthographicNear = -1.0f;
		float OrthographicFar = 1.0f;

		float PerspectiveFOV = 45.0f;
		float PerspectiveNear = 0.01f;
		float PerspectiveFar = 100.0f;

		std::unique_ptr<OrthographicCamera> orthoCamera;
		std::unique_ptr<PerspectiveCamera> perspCamera;

		CameraComponent();
		CameraComponent(const CameraComponent& other);
		CameraComponent(CameraType type);
		void UpdateCamera();
	};*/

}
