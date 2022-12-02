#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "utils/scenedata.h"

namespace Intersection {
    using Intersection = std::tuple<float, glm::vec3, std::tuple<float, float>>;
    using MaterialIntersection = std::tuple<Intersection, const SceneMaterial&>;

    bool closer(const Intersection& i1, const Intersection& i2);
    Intersection& getFirst(std::vector<Intersection>& intersections);
    MaterialIntersection& getFirst(std::vector<MaterialIntersection>& intersections);
}
