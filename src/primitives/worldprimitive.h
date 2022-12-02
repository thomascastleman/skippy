#pragma once

#include <functional>
#include <optional>
#include <iostream>
#include <glm/glm.hpp>

#include "raytracer/ray.h"
#include "utils/intersection.h"

namespace WorldPrimitive {
    // Signature: WorldSpaceRay -> Optional(IntersectionDist, WorldSpaceNormal)
    using Signature = auto(const Ray &worldSpaceRay)->std::optional<Intersection::MaterialIntersection>;
    using Proxy = std::function<Signature>;

    /**
     * @brief Given an object-space primitve, a cumulative transformation matrix, and a SceneMateral, returns a lambda
     * that takes in a ray and returns an optional of the first intersection (t and normal) and material if there is
     * any
     * 
     * @param objectPrimitive - some implicit function in object space
     * @param ctm - the cumulative transformation matrix for this primitive
     * @param material - the material of this primative
     * @return auto 
     */
    auto Primitive(auto&& objectPrimitive, const glm::mat4& ctm, const SceneMaterial& material) {
        glm::mat4 inverseCtm = glm::inverse(ctm);
        glm::mat3 normalTransform = glm::inverse(glm::transpose(glm::mat3(ctm)));

        return [=](const Ray &worldSpaceRay) {
            // transform the ray to object space
            Ray objectSpaceRay = Ray(worldSpaceRay);
            objectSpaceRay.transform(inverseCtm, false); // do not normalize direction in object space

            // get the intersection
            std::optional<Intersection::Intersection> objectIntersect = objectPrimitive(objectSpaceRay);

            if (!objectIntersect.has_value()) {
                return std::optional<Intersection::MaterialIntersection>(std::nullopt);
            }

            // transform the normal to world space
            auto [ t, objectNormal, uv ] = objectIntersect.value();
            glm::vec3 worldNormal = glm::normalize(normalTransform * objectNormal);

            return std::optional<Intersection::MaterialIntersection>(Intersection::MaterialIntersection{ Intersection::Intersection{ t, worldNormal, uv }, material });
        };
    }
}
