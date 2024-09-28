#include <PCH/pch.h>
#include "Entities/GameObject.h"
#include "Graphics/Vulkan/Window/Window.h"
#include "KeyboardController.h"
#include "Input/input.h"

namespace Dog {

    void KeyboardMovementController::moveInPlaneXZ(
        GLFWwindow* window, float dt, GameObject& gameObject) {
        glm::vec3 rotate{ 0 };
        if (Input::isKeyDown(Key::RIGHT)) rotate.y += 1.f;
        if (Input::isKeyDown(Key::LEFT)) rotate.y -= 1.f;
        if (Input::isKeyDown(Key::UP)) rotate.x += 1.f;
        if (Input::isKeyDown(Key::DOWN)) rotate.x -= 1.f;

        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
            gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
        }

        // limit pitch values between about +/- 85ish degrees
        gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
        gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

        float yaw = gameObject.transform.rotation.y;
        const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
        const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
        const glm::vec3 upDir{ 0.f, -1.f, 0.f };

        glm::vec3 moveDir{ 0.f };

        if (Input::isKeyDown(Key::W)) moveDir += forwardDir;
        if (Input::isKeyDown(Key::S)) moveDir -= forwardDir;
        if (Input::isKeyDown(Key::D)) moveDir += rightDir;
        if (Input::isKeyDown(Key::A)) moveDir -= rightDir;
        if (Input::isKeyDown(Key::E)) moveDir += upDir;
        if (Input::isKeyDown(Key::Q)) moveDir -= upDir;

        // check shift
        moveSpeed = (Input::isKeyDown(Key::LEFTSHIFT)) ? 8.f : 4.f;

        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
            gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
        }

        // log translation
    }

} // namespace Dog