#pragma once

#include <vector>
#include "primitives/worldprimitive.h"

namespace RayTracerHelper {
    std::vector<Intersection::MaterialIntersection> getIntersections(const Ray& ray, const std::vector<WorldPrimitive::Proxy>& prims);
    bool hasIntersection(const Ray& ray, const std::vector<WorldPrimitive::Proxy>& prims);
    bool hasIntersectionBefore(const Ray& ray,  const std::vector<WorldPrimitive::Proxy>& prims, const glm::vec3& pos);
}
