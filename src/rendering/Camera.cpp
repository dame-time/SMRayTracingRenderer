#include "Camera.hpp"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

Camera::Camera()
    : m_focus(0.0f, 0.0f, 0.0f)
    , m_distance(5.0f)
    , m_angleX(0.0f)
    , m_angleY(0.0f)
    , m_up(0.0f, 1.0f, 0.0f)
    , m_fov(45.0f)
    , m_nearPlane(0.1f)
    , m_farPlane(10000.0f)
    , m_orthoScale(5.0f)
    , m_isPerspective(true)
{
}

void Camera::setFocus(const glm::vec3& focus)
{
    m_focus = focus;
}

const glm::vec3& Camera::focus() const
{
    return m_focus;
}

void Camera::setDistance(const float distance)
{
    m_distance = glm::max(0.01f, distance);
}

float Camera::distance() const
{
    return m_distance;
}

void Camera::rotate(const float deltaX, const float deltaY)
{
    constexpr float rotationSpeed = 0.4f;
    m_angleY += deltaX * rotationSpeed;
    m_angleX += deltaY * rotationSpeed;

    if (m_angleX > 89.9f)  m_angleX = 89.9f;
    if (m_angleX < -89.9f) m_angleX = -89.9f;
}

void Camera::pan(const float deltaX, const float deltaY)
{
    const float panSpeed = 0.01f * m_distance;
    const float radX = glm::radians(m_angleX);
    const float radY = glm::radians(m_angleY);

    glm::vec3 forward;
    forward.x = (cos(radX) * cos(radY));
    forward.y = (sin(radX));
    forward.z = (cos(radX) * cos(radY));
    forward = normalize(forward);

    const glm::vec3 right = normalize(glm::cross(forward, m_up));
    const glm::vec3 cameraUp = normalize(glm::cross(right, forward));

    m_focus -= deltaX * panSpeed * right;
    m_focus += deltaY * panSpeed * cameraUp;
}

void Camera::zoom(const float deltaZoom)
{
    constexpr float zoomSpeed = 0.1f;
    if (m_isPerspective)
        setDistance(m_distance + deltaZoom * zoomSpeed * m_distance);
    else
    {
        setDistance(m_distance + deltaZoom * zoomSpeed * m_distance * 2.0f);
        setOrthoScale(m_orthoScale + deltaZoom * zoomSpeed * m_orthoScale);
    }
}
void Camera::setFov(const float fov)
{
    m_fov = fov;
}

float Camera::fov() const
{
    return m_fov;
}

void Camera::setNearPlane(const float nearPlane)
{
    m_nearPlane = glm::max(0.0001f, nearPlane);
}

float Camera::nearPlane() const
{
    return m_nearPlane;
}

void Camera::setFarPlane(const float farPlane)
{
    m_farPlane = glm::max(m_nearPlane + 0.01f, farPlane);
}

float Camera::farPlane() const
{
    return m_farPlane;
}

void Camera::setOrthoScale(const float scale)
{
    m_orthoScale = glm::max(0.01f, scale);
}

float Camera::orthoScale() const
{
    return m_orthoScale;
}

void Camera::setPerspective(const bool perspective)
{
    m_isPerspective = perspective;
}

bool Camera::isPerspective() const
{
    return m_isPerspective;
}

void Camera::toggleProjection()
{
    m_isPerspective = !m_isPerspective;
}

glm::mat4 Camera::viewMatrix() const
{
    const float radX = glm::radians(m_angleX);
    const float radY = glm::radians(m_angleY);

    const float x = m_focus.x + m_distance * cos(radX) * sin(radY);
    const float y = m_focus.y + m_distance * sin(radX);
    const float z = m_focus.z + m_distance * cos(radX) * cos(radY);

    return lookAt({x, y, z}, m_focus, m_up);
}

glm::mat4 Camera::projectionMatrix(const float aspect) const
{
    glm::mat4 proj;
    if (m_isPerspective)
        proj = glm::perspective(m_fov, aspect, m_nearPlane, m_farPlane);
    else
    {
        const float orthoWidth = m_orthoScale * aspect;
        const float orthoHeight = m_orthoScale;
        proj = glm::ortho(-orthoWidth, orthoWidth,
            -orthoHeight, orthoHeight, m_nearPlane, m_farPlane);
    }
    return proj;
}
