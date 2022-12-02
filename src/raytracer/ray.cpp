#include "ray.h"

/**
 * @brief Construct a new Ray:: Ray object
 * 
 * @param p - The origin position of the ray
 * @param d - The direction of the ray
 */
Ray::Ray(glm::vec3 p, glm::vec3 d) {
    m_p = p;
    m_d = d;
}

/**
 * @brief Construct a new Ray:: Ray object based off another ray object
 * 
 * @param ray - constructs a copy of the passed in ray object
 */
Ray::Ray(const Ray &ray) {
    m_p = glm::vec3(ray.getPos());
    m_d = glm::vec3(ray.getDir());
}

/**
 * @brief Get the position of the ray
 * 
 * @return const glm::vec3& - reference to the ray's position
 */
const glm::vec3 &Ray::getPos() const {
    return m_p;
}

/**
 * @brief Get the direction of the ray
 * 
 * @return const glm::vec3& - reference to the ray's direction vector
 */
const glm::vec3 &Ray::getDir() const {
    return m_d;
}

/**
 * @brief Given some distance t find the 3d point the vector reaches at that distance
 * 
 * @param t - some distance along the ray
 * @return const glm::vec3 - the point the ray reaches at that distance
 */
const glm::vec3 Ray::getPoint(const float &t) const {
    return m_p + m_d * t;
}

/**
 * @brief Transform the ray into some other space using a transformation matrix
 * 
 * @param transformationMatrix - a 4d transformation matrix
 * @param normalizeDir - a boolean (defaulted to true) denoting whether to normalize the direction after transformation
 */
void Ray::transform(const glm::mat4 &transformationMatrix, bool normalizeDir) {
    glm::vec4 p_4d = glm::vec4(m_p, 1.0f);
    glm::vec4 d_4d = glm::vec4(m_d, 0.0f);

    m_p = glm::vec3(transformationMatrix * p_4d);
    m_d =  glm::vec3(transformationMatrix * d_4d);

    if (normalizeDir) {
       m_d = glm::normalize(m_d);
    };
}



