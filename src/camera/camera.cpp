#include <stdexcept>
#include "camera.h"

/**
 * @brief Camera::Camera - Initialize the camera given some SceneCameraData
 * @param data - A reference to some SceneCameraData
 */
Camera::Camera(const SceneCameraData &data, float aspectRatio):
    m_data(data),
    m_aspectRatio(aspectRatio),
    m_viewMatrix(calculateViewMatrix(data.look, data.up, data.pos)),
    m_inverseViewMatrix(glm::inverse(m_viewMatrix))
{}

/**
 * @brief calculateViewMatrix - Returns a fully formed view matrix given the look, up, and pos vectors of the camera
 * @param look -3d vector of where the camera is looking
 * @param up - 3d vector that is not parallel to look
 * @param pos - 3d vector that is the x, y, z position of camera in world space
 * @return A 4d matrix representing the view matrix of the camera
 */
glm::mat4 Camera::calculateViewMatrix(const glm::vec3 &look, const glm::vec3 &up, const glm::vec3 &pos) {
    glm::vec3 w = glm::normalize(-look);
    glm::vec3 v = glm::normalize(up - glm::dot(up, w)*w);
    glm::vec3 u = glm::cross(v, w);

    glm::vec4 xRotateCol = glm::vec4(u.x, v.x, w.x, 0);
    glm::vec4 yRotateCol = glm::vec4(u.y, v.y, w.y, 0);
    glm::vec4 zRotateCol = glm::vec4(u.z, v.z, w.z, 0);
    glm::vec4 wRotateCol = glm::vec4(0, 0, 0, 1);

    glm::mat4 rotateM = glm::mat4(xRotateCol, yRotateCol, zRotateCol, wRotateCol);
    glm::mat4 translateM = glm::translate(-pos);

    return rotateM * translateM;
}

/**
 * @brief Camera::getViewMatrix - returns the current view matrix of the camera
 * @return a 4d matrix representing the current view matrix of the camera
 */
glm::mat4 Camera::getViewMatrix() const {
    return m_viewMatrix;
}

/**
 * @brief Camera::getInverseViewMatrix - returns the current inverse view matrix of the camera
 * @return a 4d matrix representing the current inverse view matrix of the camera
 */
glm::mat4 Camera::getInverseViewMatrix() const {
    return m_inverseViewMatrix;
}

/**
 * @brief Camera::getAspectRatio - get the aspect ratio of the camera
 * @return - float of aspect ration of the camera
 */
float Camera::getAspectRatio() const {
    return m_aspectRatio;
}

/**
 * @brief Camera::getHeightAngle - get the height angle of the camera
 * @return - float height angle in radians of camera
 */
float Camera::getHeightAngle() const {
    return m_data.heightAngle;
}

/**
 * @brief Camera::getFocalLength - get the focal length of the camera
 * @return - float focal length of the camerq
 */
float Camera::getFocalLength() const {
    return m_data.focalLength;
}

/**
 * @brief Camera::getAperture - get the aperture of the camera
 * @return - float aperture of the camera
 */
float Camera::getAperture() const {
    return m_data.aperture;
}
