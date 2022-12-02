#pragma once

#include <functional>
#include <glm/glm.hpp>
#include <tuple>
#include <iostream>

#include "primitives/worldprimitive.h"
#include "raytracer/raytracerhelper.h"

static const float EPSILON = 0.001f;

static auto createAttenuation(const glm::vec3& func) {
    return [=](float d) {
        return fmin(1.f, 1.f / (func.z * powf(d, 2) + func.y * d + func.x));
    };
}

namespace Lights {
    // IntersectionPoint, WorldPrimitives, EnableShadows -> (LightDirection, LightColor, Visible)
    using Signature = auto(const glm::vec3& intersection, const std::vector<WorldPrimitive::Proxy>& prims, bool enableShadows)->std::tuple<glm::vec3, glm::vec4, bool>;
    using Proxy = std::function<Signature>;

    auto Directional(auto&& direction, auto&& color) {
        return [=](const glm::vec3& intersection, const std::vector<WorldPrimitive::Proxy>& prims, bool enableShadows) {
            glm::vec3 lightToIntersect = glm::normalize(direction);
            glm::vec3 intersectToLight = -lightToIntersect;

            bool visible = true;
            if (enableShadows) {
                Ray visibilityRay = Ray(intersection + intersectToLight * EPSILON, intersectToLight);
                visible = !RayTracerHelper::hasIntersection(visibilityRay, prims);
            }

            return std::tuple<glm::vec3, glm::vec4, bool>{ lightToIntersect, color, visible };
        };
    }

    auto Point(auto&& pos4d, auto&& func, auto&& color) {
        auto attFunc = createAttenuation(func);
        glm::vec3 pos = glm::vec3(pos4d);

        return [=](const glm::vec3& intersection, const std::vector<WorldPrimitive::Proxy>& prims, bool enableShadows) {
            float att = attFunc(glm::distance(intersection, pos));
            glm::vec3 lightToIntersect = glm::normalize(intersection - pos);
            glm::vec3 intersectToLight = -lightToIntersect;

            // detect whether the light is even visible from the intersection point
            bool visible = true;
            if (enableShadows) {
                Ray visibilityRay = Ray(intersection + intersectToLight * EPSILON, intersectToLight);
                visible = !RayTracerHelper::hasIntersectionBefore(visibilityRay, prims, pos);
            }

            return std::tuple<glm::vec3, glm::vec4, bool>{ lightToIntersect, att * color, visible };
        };
    }

    auto Spot(auto&& pos4d, auto&& direction, auto&& outerAngle, auto&& penumbra, auto&& func, auto&& color) {
        float innerAngle = outerAngle - penumbra;

        // A cubic fallof function that given some angle returns a value between 0-1
        auto falloff = [=](float x) {
            float var = (x - innerAngle) / penumbra;
            return -2.f * powf(var, 3) + 3.f * powf(var, 2);
        };

        auto attFunc = createAttenuation(func);

        glm::vec3 pos = glm::vec3(pos4d);
        glm::vec3 spotDir = glm::normalize(direction);

        return [=](const glm::vec3& intersection, const std::vector<WorldPrimitive::Proxy>& prims, bool enableShadows) {
           glm::vec3 dir = glm::normalize(intersection - pos);

           glm::vec3 lightToIntersect = glm::normalize(intersection - pos);
           glm::vec3 intersectToLight = -lightToIntersect;

           // detect whether the lught is even visible from the intersection point
           bool visible = true;
           if (enableShadows) {
               Ray visibilityRay = Ray(intersection + intersectToLight * EPSILON, intersectToLight);
               visible = !RayTracerHelper::hasIntersectionBefore(visibilityRay, prims, pos);
           }

           float angle = acosf(glm::dot(spotDir, dir));
           float att = attFunc(glm::distance(intersection, pos));

           // in the inner circle, it is at full color
           if (angle <= innerAngle) {
               return std::tuple<glm::vec3, glm::vec4, bool>{ dir, att * color, visible };
           }

           // if the angle is too large, the spotlight has no color
           if (angle > outerAngle) {
               return std::tuple<glm::vec3, glm::vec4, bool>{ dir, glm::vec4(0.f), visible };
           }

            // between the inner and outer cone the color is given by some falloff function.
           return std::tuple<glm::vec3, glm::vec4, bool>{ dir, (1 - falloff(angle)) * att * color, visible };
        };
    }
}
