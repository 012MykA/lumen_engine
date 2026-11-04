#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace lm {

class Event;

class Camera {
public:
	virtual ~Camera() = default;

	virtual void OnUpdate(float ts) = 0;
	virtual void OnEvent(Event &) {}

public:
	const glm::vec3 &GetPosition() const { return m_Position; }
	const glm::mat4 &GetView() const;
	const glm::mat4 &GetProjection() const;
	const glm::mat4 &GetViewProjection() const;

	void SetPosition(const glm::vec3 &position);
	void SetAspectRatio(float aspect);

protected:
	virtual void UpdateViewMatrix() const = 0;
	virtual void UpdateProjectionMatrix() const = 0;

protected:
	glm::vec3 m_Position{ 0.0f };

	mutable glm::mat4 m_ViewMatrix{ 1.0f };
	mutable glm::mat4 m_ProjectionMatrix{ 1.0f };
	mutable glm::mat4 m_ViewProjectionMatrix{ 1.0f };

	mutable bool m_IsViewDirty{ true };
	mutable bool m_IsProjectionDirty{ true };

	float m_AspectRatio{ 0.0f };
};

} // namespace lm
