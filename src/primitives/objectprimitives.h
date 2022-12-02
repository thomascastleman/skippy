#pragma once

#include <functional>
#include <optional>
#include <glm/glm.hpp>

#include "raytracer/ray.h"
#include "solvers.h"
#include "utils/intersection.h"

#include <iostream>

namespace ObjectPrimitives {
    // Signature: ObjectSpaceRay -> Optional(IntersectionDist, ObjectSpaceNormal)
    using Signature = auto(const Ray &objectSpaceRay)->std::optional<Intersection::Intersection>;
    using Proxy = std::function<Signature>;

    /**
     * @brief Given a vector of solver functions and a ray, returns the closest intersection to the camera calculated by the solvers
     * 
     * @param solvers - A vector of Solver implicit functions
     * @param ray - A ray object to intersect with the solvers
     * @return - An optional intersection tuple that represents the closest intersection, if there even is one
     */
    inline auto getFirstSolution(auto&& solvers, const Ray& ray) {
        float foundValidSolution = false;
        Intersection::Intersection first;

        for (const auto& solver : solvers) {
            auto curSolutions = solver(ray);
            if (curSolutions.empty()) { continue; }

            auto& curFirst = Intersection::getFirst(curSolutions);

            if (!foundValidSolution) {
                foundValidSolution = true;
                first = curFirst;
            } else if (Intersection::closer(curFirst, first)) {
                first = curFirst;
            }
        }

        if (!foundValidSolution) {
            return std::optional<Intersection::Intersection>(std::nullopt);
        }

        return std::optional<Intersection::Intersection>{ first } ;
    }

    /**
     * @brief Implicit function for a cube
     * 
     * @return A lambda that takes in a ray and returns an optional intersection tuple 
     * representing where the closest intersection on the shape was if any.
     */
    constexpr auto Cube() {
        return [=](const Ray &objectSpaceRay) {
            return getFirstSolution(Solvers::CubeSolvers, objectSpaceRay);
        };
    };

    /**
     * @brief Implicit function for a cone
     * 
     * @return A lambda that takes in a ray and returns an optional intersection tuple
     * representing where the closest intersection on the shape was if any.
     */
    constexpr auto Cone() {
        return [=](const Ray &objectSpaceRay) {
            return getFirstSolution(Solvers::ConeSolvers, objectSpaceRay);
        };
    }

    /**
     * @brief Implicit function for a cylinder
     * 
     * @return A lambda that takes in a ray and returns an optional intersection tuple
     * representing where the closest intersection on the shape was if any.
     */
    constexpr auto Cylinder() {
        return [=](const Ray &objectSpaceRay) {
            return getFirstSolution(Solvers::CylinderSolvers, objectSpaceRay);
        };
    }

    /**
     * @brief Implicit function for a sphere
     * 
     * @return A lambda that takes in a ray and returns an optional intersection tuple
     * representing where the closest intersection on the shape was if any.
     */
    constexpr auto Sphere() {
        return [=](const Ray &objectSpaceRay) {
            return getFirstSolution(Solvers::SphereSolvers, objectSpaceRay);
        };
    }
}
