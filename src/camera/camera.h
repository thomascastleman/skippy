#pragma once

#include "utils/scenedata.h"
#include "glm/gtx/transform.hpp"
#include <cmath>

// A class representing a virtual camera.

// Feel free to make your own design choices for Camera class, the functions below are all optional / for your convenience.
// You can either implement and use these getters, or make your own design.
// If you decide to make your own design, feel free to delete these as TAs won't rely on them to grade your assignments.

class Camera {
public:
    Camera(const SceneCameraData &data, float aspectRatio);

    // Returns the view matrix for the current camera settings.
    glm::mat4 getViewMatrix() const;

    // Returns the inverse view matrix for the current camera settings
    glm::mat4 getInverseViewMatrix() const;

    // Returns the aspect ratio of the camera.
    float getAspectRatio() const;

    // Returns the height angle of the camera in RADIANS.
    float getHeightAngle() const;

    // Returns the focal length of this camera.
    // This is for the depth of field extra-credit feature only;
    // You can ignore if you are not attempting to implement depth of field.
    float getFocalLength() const;

    // Returns the focal length of this camera.
    // This is for the depth of field extra-credit feature only;
    // You can ignore if you are not attempting to implement depth of field.
    float getAperture() const;
private:
   static glm::mat4 calculateViewMatrix(const glm::vec3 &look, const glm::vec3 &up, const glm::vec3 &pos);

   const  SceneCameraData& m_data;
   const float m_aspectRatio;
   const glm::mat4 m_viewMatrix;
   const glm::mat4 m_inverseViewMatrix;
};
