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
	if (m_IsViewProjDirty) {
		m_ViewProjectionMatrix = GetProjection() * GetView();
		m_IsViewProjDirty = false;
	}
	return m_ViewProjectionMatrix;
}

void Camera::SetPosition(const glm::vec3 &pos) {
	m_Position = pos;
	SetViewDirty();
}

void Camera::SetAspectRatio(float aspect) {
	m_AspectRatio = aspect;
	SetProjectionDirty();
}

void Camera::SetViewDirty(bool dirty) {
	m_IsViewDirty = dirty;
	if (dirty) {
		m_IsViewProjDirty = true;
	}
}

void Camera::SetProjectionDirty(bool dirty) {
	m_IsProjectionDirty = dirty;
	if (dirty) {
		m_IsViewProjDirty = true;
	}
}

} // namespace lm
