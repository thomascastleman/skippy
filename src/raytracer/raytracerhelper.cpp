#include "raytracerhelper.h"

/**
 * @brief RayTracerHelper::getIntersections - Gets a list of all valid intersections given a ray and a vector of world primitives
 * @param ray - a ray in world space
 * @param prims - a vector of world primitives to detect intersections with
 * @return A vector of all the intersections
 */
std::vector<Intersection::MaterialIntersection> RayTracerHelper::getIntersections(const Ray& ray,  const std::vector<WorldPrimitive::Proxy>& prims) {
    std::vector<Intersection::MaterialIntersection> intersections;
    for (auto &prim : prims) {
        std::optional<Intersection::MaterialIntersection> materialIntersection = prim(ray);

        if (materialIntersection.has_value()) {
            intersections.push_back(materialIntersection.value());
        }
    }
    return intersections;
}

/**
 * @brief RayTracerHelper::hasIntersection - Tells whether there is a valid intersection given a ray and some world primitives
 * @param ray - a ray in world space
 * @param prims - a vector of world primitives to detect intersections with
 * @return A boolean denoting whether there is 1 or more valid intersections.
 */
bool RayTracerHelper::hasIntersection(const Ray& ray,  const std::vector<WorldPrimitive::Proxy>& prims) {
    std::vector<Intersection::MaterialIntersection> intersections = getIntersections(ray, prims);

    return !intersections.empty();
}

/**
 * @brief RayTracerHelper::hasIntersectionBefore - given a ray, some primitives, and a position, tells whether there is a valid intersection
 * before the given position
 * @param ray - a ray in world space
 * @param prims - a vector of world primitives to detect intersections with
 * @param pos - some 3d position (along ray) to detect if there are closer intersections
 * @return A boolean denoting whether there are any intersections closer to the origin of the ray than the provided position
 */
bool RayTracerHelper::hasIntersectionBefore(const Ray& ray,  const std::vector<WorldPrimitive::Proxy>& prims, const glm::vec3& pos) {
    std::vector<Intersection::MaterialIntersection> intersections = getIntersections(ray, prims);

    float posT = ((pos - ray.getPos()) / ray.getDir())[0];
    for (auto& matIntersection : intersections) {
        auto& [intersection, mat] = matIntersection;
        auto& [t, norm, uv] = intersection;

        if (t < posT) {
            return true;
        }
    }

    return false;
}


