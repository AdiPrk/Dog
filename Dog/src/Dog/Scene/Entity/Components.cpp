#include <PCH/pch.h>
#include "components.h"
#include "Dog/engine.h"
#include "Dog/Graphics/Vulkan/Window/Window.h"
#include "Dog/Graphics/Vulkan/Texture/Texture.h"
#include "Dog/Graphics/Vulkan/Models/ModelLibrary.h"
#include "Dog/Graphics/Vulkan/Models/Model.h"

namespace Dog {

	glm::mat4 TransformComponent::mat4() const {
		const float c3 = glm::cos(Rotation.z);
		const float s3 = glm::sin(Rotation.z);
		const float c2 = glm::cos(Rotation.x);
		const float s2 = glm::sin(Rotation.x);
		const float c1 = glm::cos(Rotation.y);
		const float s1 = glm::sin(Rotation.y);
		return glm::mat4{
			{
				Scale.x * (c1 * c3 + s1 * s2 * s3),
				Scale.x * (c2 * s3),
				Scale.x * (c1 * s2 * s3 - c3 * s1),
				0.0f,
			},
			{
				Scale.y * (c3 * s1 * s2 - c1 * s3),
				Scale.y * (c2 * c3),
				Scale.y * (c1 * c3 * s2 + s1 * s3),
				0.0f,
			},
			{
				Scale.z * (c2 * s1),
				Scale.z * (-s2),
				Scale.z * (c1 * c2),
				0.0f,
			},
			{Translation.x, Translation.y, Translation.z, 1.0f} };
	}

	glm::mat3 TransformComponent::normalMatrix() const {
		const float c3 = glm::cos(Rotation.z);
		const float s3 = glm::sin(Rotation.z);
		const float c2 = glm::cos(Rotation.x);
		const float s2 = glm::sin(Rotation.x);
		const float c1 = glm::cos(Rotation.y);
		const float s1 = glm::sin(Rotation.y);
		const glm::vec3 invScale = 1.0f / Scale;

		return glm::mat3{
			{
				invScale.x * (c1 * c3 + s1 * s2 * s3),
				invScale.x * (c2 * s3),
				invScale.x * (c1 * s2 * s3 - c3 * s1),
			},
			{
				invScale.y * (c3 * s1 * s2 - c1 * s3),
				invScale.y * (c2 * c3),
				invScale.y * (c1 * c3 * s2 + s1 * s3),
			},
			{
				invScale.z * (c2 * s1),
				invScale.z * (-s2),
				invScale.z * (c1 * c2),
			},
		};
	}

	SpriteComponent::SpriteComponent()
		: Color(1.0f)
		, texturePath("square.png")
	{
	}

	SpriteComponent::SpriteComponent(const glm::vec4& color, const std::string& texturePath)
		: Color(color)
		, texturePath(texturePath)
	{
	}

	/*void CameraComponent::SetAsCurrentCamera()
	{
		//
	}

	CameraComponent::CameraComponent()
	{
		float width = (float)Engine::Get().GetWindow().GetWidth();
		float height = (float)Engine::Get().GetWindow().GetHeight();
		float aspectRatio = width / height;

		//orthoCamera = std::make_unique<OrthographicCamera>(aspectRatio, OrthographicSize);
		//perspCamera = std::make_unique<PerspectiveCamera>();
	}

	CameraComponent::CameraComponent(const CameraComponent& other)
		: Projection(other.Projection)
		, OrthographicSize(other.OrthographicSize)
		, OrthographicNear(other.OrthographicNear)
		, OrthographicFar(other.OrthographicFar)
		, PerspectiveFOV(other.PerspectiveFOV)
		, PerspectiveNear(other.PerspectiveNear)
		, PerspectiveFar(other.PerspectiveFar)
		//, orthoCamera(std::make_unique<OrthographicCamera>(*other.orthoCamera))
		//, perspCamera(std::make_unique<PerspectiveCamera>(*other.perspCamera))
	{
	}

	CameraComponent::CameraComponent(CameraType type)
		: Projection(type)
	{
		float width = (float)Engine::Get().GetWindow().GetWidth();
		float height = (float)Engine::Get().GetWindow().GetHeight();
		float aspectRatio = width / height;

		//orthoCamera = std::make_unique<OrthographicCamera>(aspectRatio, OrthographicSize);
		//perspCamera = std::make_unique<PerspectiveCamera>();
	}

	void CameraComponent::UpdateCamera()
	{

	}*/

	ModelComponent::ModelComponent(const ModelComponent&)
	{
	}

	ModelComponent::ModelComponent(const std::string& modelPath)
		: modelPath(modelPath)
	{
		// Get the model's index from the model library
		modelIndex = Engine::Get().GetModelLibrary().AddModel(modelPath);
		DOG_INFO("ModelComponent: Model path: {0}, Model index: {1}", modelPath, modelIndex);
	}

} // namespace Dog
