#pragma once

#include <glm/glm.hpp>

/**
 * @brief A simple orbit camera around a focus point.
 *
 * This camera can switch between perspective and orthographic projections.
 */
class Camera
{
public:
	Camera();

	void setFocus(const glm::vec3 &focus);
	const glm::vec3 &focus() const;

	void setDistance(float distance);
	float distance() const;

	void rotate(float deltaX, float deltaY);
	void pan(float deltaX, float deltaY);
	void zoom(float deltaZoom);

	void setFov(float fov);
	float fov() const;

	void setNearPlane(float nearPlane);
	float nearPlane() const;

	void setFarPlane(float farPlane);
	float farPlane() const;

	void setOrthoScale(float scale);
	float orthoScale() const;

	void setPerspective(bool perspective);
	bool isPerspective() const;
	void toggleProjection();

	glm::mat4 viewMatrix() const;
	glm::mat4 projectionMatrix(float aspect) const;

private:
	glm::vec3 m_focus;
	float m_distance;
	float m_angleX;
	float m_angleY;
	glm::vec3 m_up;

	float m_fov;
	float m_nearPlane;
	float m_farPlane;

	float m_orthoScale;

	bool m_isPerspective;
};