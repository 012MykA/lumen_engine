#pragma once

#include "lumen_engine/scene/camera.hpp"

namespace lm {

struct FPSCameraDesc {
	float fov = 70.0f;

	float nearClip = 0.01f;
	float farClip = 1000.0f;

	float movementSpeed = 2.0f;
	float sprintMultiplier = 2.0f;
	float mouseSensitivity = 0.05f;
};

struct Window;
class KeyPressedEvent;
class WindowResizeEvent;

class FPSCamera : public Camera {
public:
	FPSCamera(const Window &window, const FPSCameraDesc &desc = {});

	void OnUpdate(float ts) override;
	void OnEvent(Event &e) override;

public:
	// clang-format off
    void SetYaw(float yaw)     { m_Yaw = yaw;     UpdateCameraVectors(); SetViewDirty(); }
    void SetPitch(float pitch) { m_Pitch = pitch; UpdateCameraVectors(); SetViewDirty(); }
	// clang-format on

protected:
	void UpdateViewMatrix() const override;
	void UpdateProjectionMatrix() const override;

private:
	void UpdateCameraVectors();

	bool OnKeyPressed(KeyPressedEvent &e);
	bool OnWindowResize(WindowResizeEvent &e);

private:
	const Window &m_Window;
	FPSCameraDesc m_Desc;

	float m_Yaw{ -90.0f };
	float m_Pitch{ 0.0f };

	glm::vec3 m_Front{ 0.0f, 0.0f, -1.0f };
	glm::vec3 m_Up{ 0.0f, 1.0f, 0.0f };
	glm::vec3 m_Right{ 1.0f, 0.0f, 0.0f };
	glm::vec3 m_WorldUp{ 0.0f, 1.0f, 0.0f };

	std::pair<float, float> m_LastMouse{ 0.0f, 0.0f };
	bool m_FirstMouse{ true };
};

} // namespace lm
