#include "lumen_engine/scene/camera.hpp"

namespace lm {

const glm::mat4 &Camera::GetView() const {
	if (m_IsViewDirty) {
		UpdateViewMatrix();
		m_IsViewDirty = false;
	}
	return m_ViewMatrix;
}

const glm::mat4 &Camera::GetProjection() const {
	if (m_IsProjectionDirty) {
		UpdateProjectionMatrix();
		m_IsProjectionDirty = false;
	}
	return m_ProjectionMatrix;
}

const glm::mat4 &Camera::GetViewProjection() const {
	if (m_IsViewDirty || m_IsProjectionDirty) {
		// Updates view and proj if necessary
		m_ViewProjectionMatrix = GetProjection() * GetView();
	}
	return m_ViewProjectionMatrix;
}

void Camera::SetPosition(const glm::vec3 &position) {
	m_Position = position;
	m_IsViewDirty = true;
}

void Camera::SetAspectRatio(float aspect) {
	m_AspectRatio = aspect;
	m_IsProjectionDirty = true;
}

} // namespace lm
