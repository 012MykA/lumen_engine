#include "lumen_engine/scene/fps_camera.hpp"

#include "lumen_engine/core/events/event_dispatcher.hpp"
#include "lumen_engine/core/input/input.hpp"
#include "lumen_engine/core/window.hpp"

namespace lm {

FPSCamera::FPSCamera(const Window &window, const FPSCameraDesc &desc) :
		m_Desc(desc),
		m_Window(window) {
	auto extent = window.GetExtent();
	float aspect = static_cast<float>(extent.width) / (extent.height);
	Camera::SetAspectRatio(aspect);
	Input::SetCursorCaptured(true);

	m_LastMouse = Input::GetMousePosition();

	UpdateCameraVectors();
}

void FPSCamera::OnUpdate(float ts) {
	if (!Input::IsCursorCaptured()) {
		m_FirstMouse = true;
		return;
	}

	float sprintMultiplier = Input::IsKeyPressed(Key::LeftShift) ? m_Desc.sprintMultiplier : 1.0f;
	float velocity = m_Desc.movementSpeed * ts;
	float speed = velocity * sprintMultiplier;

	// clang-format off
    if (Input::IsKeyPressed(Key::W)) { SetPosition(GetPosition() + m_Front * speed); }
    if (Input::IsKeyPressed(Key::S)) { SetPosition(GetPosition() - m_Front * speed); }
    if (Input::IsKeyPressed(Key::A)) { SetPosition(GetPosition() - m_Right * speed); }
    if (Input::IsKeyPressed(Key::D)) { SetPosition(GetPosition() + m_Right * speed); }

    if (Input::IsKeyPressed(Key::E)) { SetPosition(GetPosition() + m_Up * speed); }
    if (Input::IsKeyPressed(Key::Q)) { SetPosition(GetPosition() - m_Up * speed); }
	// clang-format on

	auto [mouseX, mouseY] = Input::GetMousePosition();

	if (m_FirstMouse) {
		m_LastMouse = { mouseX, mouseY };
		m_FirstMouse = false;
	}

	float xOffset = mouseX - m_LastMouse.first;
	float yOffset = m_LastMouse.second - mouseY;

	m_LastMouse = { mouseX, mouseY };

	if (xOffset != 0 || yOffset != 0) {
		float sensitivity = m_Desc.mouseSensitivity;

		m_Yaw += xOffset * sensitivity;
		m_Pitch += yOffset * sensitivity;

		// clang-format off
        if (m_Pitch > 89.9f) m_Pitch = 89.9f;
        if (m_Pitch < -89.9f) m_Pitch = -89.9f;
		// clang-format on

		UpdateCameraVectors();
		m_IsViewDirty = true;
	}
}

void FPSCamera::OnEvent(Event &event) {
	EventDispatcher dp(event);
	dp.Dispatch<KeyPressedEvent>([this](auto &e) { return OnKeyPressed(e); });
}

void FPSCamera::UpdateViewMatrix() const {
	m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}

void FPSCamera::UpdateProjectionMatrix() const {
	m_ProjectionMatrix = glm::perspective(
			glm::radians(m_Desc.fov),
			Camera::m_AspectRatio,
			m_Desc.nearClip,
			m_Desc.farClip);

	// Invert Y for Vulkan
	m_ProjectionMatrix[1][1] *= -1;
}

void FPSCamera::UpdateCameraVectors() {
	glm::vec3 front;
	front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	front.y = sin(glm::radians(m_Pitch));
	front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));

	m_Front = glm::normalize(front);
	m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
	m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}

bool FPSCamera::OnKeyPressed(KeyPressedEvent &e) {
	bool isRepeat = e.GetRepeatCount() > 0;
	if (e.GetKey() == Key::Escape && !isRepeat) {
		bool isLocked = Input::IsCursorCaptured();
		Input::SetCursorCaptured(!isLocked);

		if (!isLocked) {
			m_FirstMouse = true;
		}

		return true;
	}

	return false;
}

} // namespace lm
