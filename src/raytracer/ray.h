#pragma once

#include <glm/glm.hpp>

class Ray {
public:
    Ray(glm::vec3 p, glm::vec3 d);
    Ray(const Ray &ray);
    void transform(const glm::mat4 &transformationMatrix,  bool normalizeDir = true);
    const glm::vec3 &getPos() const;
    const glm::vec3 &getDir() const;
    const glm::vec3 getPoint(const float& t) const;

private:
    glm::vec3 m_p;
    glm::vec3 m_d;
};

