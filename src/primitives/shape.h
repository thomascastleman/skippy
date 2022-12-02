#pragma once

#include "raytracer/ray.h"
#include <any>
#include <optional>
#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include "utils/scenedata.h"

/*

THIS IS A LEGACY FILE
- this file is from my first attempt at building the ray tracer
- it worked but was super messy so I decided to adopt implicit functions
- just kept it around because I thought it was neat!
- does not get compiled

*/


const float EPSILON = 0.000001f;

// Gotten from the CS123 helper
template<typename UnknownType, typename ReferenceType>
concept SubtypeOf = std::same_as<std::decay_t<UnknownType>, ReferenceType> || std::derived_from<std::decay_t<UnknownType>, ReferenceType>;

template<typename UnknownType, typename ...ReferenceTypes>
concept AnyOf = (SubtypeOf<UnknownType, ReferenceTypes> || ...);

template<typename UnknownType, typename ...ReferenceTypes>
concept AnyBut = !AnyOf<UnknownType, ReferenceTypes...>;
// end CS123 helper


static float getDescriminant(float a, float b, float c) {
    return powf(b, 2) - (4 * a * c);
}

static std::vector<float> solveQuadratic(float a, float b, float c) {
    std::vector<float> solutions;

    float descriminant = getDescriminant(a, b, c);

    if (glm::epsilonEqual(a, 0.f, EPSILON)){
        return solutions;
    }

    if (descriminant >= 0.f) {
        solutions.push_back((-b + sqrt(descriminant)) / (2 * a));
    }

    if (descriminant > 0.f) {
        solutions.push_back((-b - sqrt(descriminant)) / (2 * a));
    }

    return solutions;
}

static bool inSquarePlaneBounds(float horizontalPos, float verticalPos) {
   return horizontalPos >= -0.5f && horizontalPos <= 0.5f && verticalPos >= -0.5f && verticalPos <= 0.5f;
}

struct Cone {
    const glm::mat4 &ctm;
    const glm::mat4 inverseCtm;
    const glm::mat3 normalTransformCtm;
    const SceneMaterial& material;

    Cone(const glm::mat4 &ctm, const SceneMaterial& material):
        ctm(ctm),
        inverseCtm(glm::inverse(ctm)),
        normalTransformCtm(glm::inverse(glm::transpose(glm::mat3(ctm)))),
        material(material)
    {}

    glm::vec3 normalAt(const Ray& ray, float t) const {
        const glm::vec3 pt = ray.getPoint(t);

        // if touching the bottom cap
        if (glm::epsilonEqual(pt.y, -0.5f, EPSILON)) {
            return glm::vec3(0, -1, 0);
        }

        // if on the cone
        float xNorm = (2 * pt.x);
        float yNorm = -(1.f/4.f) * (2 * pt.y - 1);
        float zNorm = (2 * pt.z);

        return glm::normalize(normalTransformCtm * glm::vec3(xNorm, yNorm, zNorm));
    }

    std::optional<std::tuple<float, glm::vec3>> intersectWith(const Ray &ray) const {
        std::vector<float> solutions;

        // convert the ray to object space
        Ray objectSpaceRay = Ray(ray);
        objectSpaceRay.transform(inverseCtm);

        const glm::vec3 &p = objectSpaceRay.getPos();
        const glm::vec3 &d = objectSpaceRay.getDir();

        // solve for cone intersections
        float a = powf(d.x, 2) + powf(d.z, 2) - (1.f/4.f) * powf(d.y, 2);
        float b = (2 * p.x * d.x) + (2 * p.z * d.z) + ((1.f/4.f) * d.y) - ((1.f/2.f) * p.y * d.y);
        float c = powf(p.x, 2) + powf(p.z, 2) + ((1.f/ 4.f) * p.y) - ((1.f/4.f) * powf(p.y, 2)) - (1.f/16.f);

        std::vector<float> coneSolutions = solveQuadratic(a, b, c);
        for (float& sol : coneSolutions) {
            float solYPos = p.y + d.y * sol;
            if (sol >= 0 && solYPos > -0.5f && solYPos <= 0.5f) {
                solutions.push_back(sol);
            }
        }

        // solve for bottom cap intersections
        if (d.y != 0) {
            float capSolution = (-0.5f - p.y) / d.y;
            if (capSolution >= 0 && powf(p.x + d.x * capSolution, 2) + powf(p.z + d.z * capSolution, 2) <= powf(0.5, 2)) {
               solutions.push_back(capSolution);
            }
        }

        if (solutions.empty()) {
            return std::nullopt;
        }

        // find smallest intersect
        float firstIntersect = solutions[0];
        for (float& sol : solutions) {
            if (sol < firstIntersect) {
                firstIntersect = sol;
            }
        }

        return std::tuple<float, glm::vec3>{ firstIntersect, normalAt(objectSpaceRay, firstIntersect) };
    }

    SceneMaterial getMaterial() const {
        return material;
    }
};

struct Sphere {
    const glm::mat4 &ctm;
    const glm::mat4 inverseCtm;
    const glm::mat3 normalTransformCtm;
    const SceneMaterial& material;

    Sphere(const glm::mat4 &ctm, const SceneMaterial& material):
        ctm(ctm),
        inverseCtm(glm::inverse(ctm)),
        normalTransformCtm(glm::inverse(glm::transpose(glm::mat3(ctm)))),
        material(material)
    {}

    glm::vec3 normalAt(const Ray& ray, float t) const {
        const glm::vec3 pt = ray.getPoint(t);

        // if on the cone
        float xNorm = (2 * pt.x);
        float yNorm =(2 * pt.y);
        float zNorm = (2 * pt.z);

        return glm::normalize(normalTransformCtm *glm::vec3(xNorm, yNorm, zNorm));
    }

    std::optional<std::tuple<float, glm::vec3>> intersectWith(const Ray &ray) const {

        // convert the ray to object space
        Ray objectSpaceRay = Ray(ray);
        objectSpaceRay.transform(inverseCtm);

        const glm::vec3 &p = objectSpaceRay.getPos();
        const glm::vec3 &d = objectSpaceRay.getDir();

        // solve for sphere intersections
        float a = powf(d.x, 2) + powf(d.y, 2) + powf(d.z, 2);
        float b = 2 * (p.x * d.x + p.y * d.y + p.z * d.z);
        float c = powf(p.x, 2) + powf(p.y, 2) + powf(p.z, 2) - powf(1.f/2.f, 2);

        std::vector<float> solutions = solveQuadratic(a, b, c);
        bool foundValidSolution = false;
        float firstIntersect;
        for (float& sol : solutions) {
            if (sol >= 0.f) {
                if (!foundValidSolution) {
                    foundValidSolution = true;
                    firstIntersect = sol;
                } else if (sol < firstIntersect) {
                    firstIntersect = sol;
                }
            }
        }

        if (!foundValidSolution) {
            return std::nullopt;
        }

        return std::tuple<float, glm::vec3>{ firstIntersect, normalAt(objectSpaceRay, firstIntersect) };
    }

    SceneMaterial getMaterial() const {
        return material;
    }
};

struct Cylinder {
    const glm::mat4 &ctm;
    const glm::mat4 inverseCtm;
    const glm::mat3 normalTransformCtm;
    const SceneMaterial& material;

    Cylinder(const glm::mat4 &ctm, const SceneMaterial& material):
        ctm(ctm),
        inverseCtm(glm::inverse(ctm)),
        normalTransformCtm(glm::inverse(glm::transpose(glm::mat3(ctm)))),
        material(material)
    {}

    glm::vec3 normalAt(const Ray& ray, float t) const {
        const glm::vec3 pt = ray.getPoint(t);

        if (glm::epsilonEqual(pt.y, -0.5f, EPSILON)) {
            return glm::vec3(0, -1, 0);
        }

        if (glm::epsilonEqual(pt.y, 0.5f, EPSILON)) {
            return glm::vec3(0, 1, 0);
        }

        // if on the cone
        float xNorm = (2 * pt.x);
        float yNorm = 0;
        float zNorm = (2 * pt.z);

        return glm::normalize(normalTransformCtm * glm::vec3(xNorm, yNorm, zNorm));
    }

    std::optional<std::tuple<float, glm::vec3>> intersectWith(const Ray &ray) const {
        std::vector<float> solutions;

        // convert the ray to object space
        Ray objectSpaceRay = Ray(ray);
        objectSpaceRay.transform(inverseCtm);

        const glm::vec3 &p = objectSpaceRay.getPos();
        const glm::vec3 &d = objectSpaceRay.getDir();

        // solve for cone intersections
        float a = powf(d.x, 2) + powf(d.z, 2);
        float b = 2 * (p.x * d.x + p.z * d.z);
        float c = powf(p.x, 2) + powf(p.z, 2) - 0.25f;

        std::vector<float> sphereSolutions = solveQuadratic(a, b, c);
        for (float& sol : sphereSolutions) {
            float solYPos = p.y + d.y * sol;
            if (sol >= 0.f && solYPos > -0.5f && solYPos < 0.5f) {
                solutions.push_back(sol);
            }
        }

        if (d.y != 0) {
            // solve for bottom cap intersections
            float bottomCapSolution = (-0.5f - p.y) / d.y;
            if (bottomCapSolution >= 0.f && powf(p.x + d.x * bottomCapSolution, 2) + powf(p.z + d.z * bottomCapSolution, 2) <= 0.25f) {
               solutions.push_back(bottomCapSolution);
            }

            // solve for top cap intersections
            float topCapSolution = (0.5f - p.y) / d.y;
            if (topCapSolution >= 0.f && powf(p.x + d.x * topCapSolution, 2) + powf(p.z + d.z * topCapSolution, 2) <= 0.25f) {
                solutions.push_back(topCapSolution);
            }
        }

        if (solutions.empty()) {
            return std::nullopt;
        }

        // find smallest intersect
        float firstIntersect = solutions[0];
        for (float& sol : solutions) {
            if (sol < firstIntersect) {
                firstIntersect = sol;
            }
        }

        return std::tuple<float, glm::vec3>{ firstIntersect, normalAt(objectSpaceRay, firstIntersect) };
    }

    SceneMaterial getMaterial() const {
        return material;
    }
};

struct Cube {
    const glm::mat4 &ctm;
    const glm::mat4 inverseCtm;
    const glm::mat3 normalTransformCtm;
    const SceneMaterial &material;

    Cube(const glm::mat4 &ctm, const SceneMaterial &material):
        ctm(ctm),
        inverseCtm(glm::inverse(ctm)),
        normalTransformCtm(glm::inverse(glm::transpose(glm::mat3(ctm)))),
        material(material)
    {}

    glm::vec3 normalAt(const Ray& ray, float t) const {
        const glm::vec3 pt = ray.getPoint(t);

        glm::vec3 norm;
        // bottom plane
        if (glm::epsilonEqual(pt.y, -0.5f, EPSILON)) {
            norm = glm::vec3(0, -1, 0);
        }

        // top plane
        else if (glm::epsilonEqual(pt.y, 0.5f, EPSILON)) {
            norm = glm::vec3(0, 1, 0);
        }

        // left plane
        else if (glm::epsilonEqual(pt.x, -0.5f, EPSILON)) {
            norm =  glm::vec3(-1, 0, 0);
        }

        // right plane
        else if (glm::epsilonEqual(pt.x, 0.5f, EPSILON)) {
            norm = glm::vec3(1, 0, 0);
        }

        // back plane
        else if (glm::epsilonEqual(pt.z, -0.5f, EPSILON)) {
            norm = glm::vec3(0, 0, -1);
        }

        // front plane
        else {
            norm = glm::vec3(0, 0, 1);
        }

        return glm::normalize(normalTransformCtm * norm);
    }

    std::optional<std::tuple<float, glm::vec3>> intersectWith(const Ray &ray) const {
        std::vector<float> solutions;

        // convert the ray to object space
        Ray objectSpaceRay = Ray(ray);
        objectSpaceRay.transform(inverseCtm);

        const glm::vec3 &p = objectSpaceRay.getPos();
        const glm::vec3 &d = objectSpaceRay.getDir();

        // solve for bottom plane intersections
        float bottomPlaneSolution = (-0.5f - p.y) / d.y;
        if (bottomPlaneSolution >= 0.f && inSquarePlaneBounds(p.x + d.x * bottomPlaneSolution, p.z + d.z * bottomPlaneSolution)) {
           solutions.push_back(bottomPlaneSolution);
        }

        // solve for top plane intersections
        float topPlaneSolution = (0.5f - p.y) / d.y;
        if (topPlaneSolution >= 0.f && inSquarePlaneBounds(p.x + d.x * topPlaneSolution, p.z + d.z * topPlaneSolution)) {
            solutions.push_back(topPlaneSolution);
        }

        // solve for left plane intersections
        float leftPlaneSolution = (-0.5f - p.x) / d.x;
        if (leftPlaneSolution >= 0.f && inSquarePlaneBounds(p.z + d.z * leftPlaneSolution, p.y + d.y * leftPlaneSolution)) {
           solutions.push_back(leftPlaneSolution);
        }

        // solve for right plane intersections
        float rightPlaneSolution = (0.5f - p.x) / d.x;
        if (rightPlaneSolution >= 0.f && inSquarePlaneBounds(p.z + d.z * rightPlaneSolution, p.y + d.y * rightPlaneSolution)) {
            solutions.push_back(rightPlaneSolution);
        }

        // solve for back plane intersections
        float backPlaneSolution = (-0.5f - p.z) / d.z;
        if (backPlaneSolution >= 0.f && inSquarePlaneBounds(p.x + d.x * backPlaneSolution, p.y + d.y * backPlaneSolution)) {
           solutions.push_back(backPlaneSolution);
        }

        // solve for right plane intersections
        float frontPlaneSolution = (0.5f - p.z) / d.z;
        if (frontPlaneSolution >= 0.f && inSquarePlaneBounds(p.x + d.x * frontPlaneSolution, p.y + d.y * frontPlaneSolution)) {
            solutions.push_back(frontPlaneSolution);
        }

        if (solutions.empty()) {
            return std::nullopt;
        }

        // find smallest intersect
        float firstIntersect = solutions[0];
        for (float& sol : solutions) {
            if (sol < firstIntersect) {
                firstIntersect = sol;
            }
        }

        return std::tuple<float, glm::vec3>{ firstIntersect, normalAt(objectSpaceRay, firstIntersect) };
    }

    SceneMaterial getMaterial() const {
        return material;
    }
};

struct Shape {
    using IntersectWithSignature = auto(const std::any&, const Ray&)->std::optional<std::tuple<float, glm::vec3>>;
    using GetMaterialSignature = auto(const std::any&)->SceneMaterial;

    std::any object = {};
    IntersectWithSignature* intersectWithFunction = nullptr;
    GetMaterialSignature* getMaterialFunction = nullptr;

    Shape() = default;

    // exclude "Shape" itself from the universal quantification
    // to avoid conflict with the copy/move constructors
    Shape(const AnyBut<Shape> auto&& x) {
        using ObjectType = std::decay_t<decltype(x)>;

        this->object = std::forward<decltype(x)>(x);
        this->intersectWithFunction = [](const auto& object, const Ray& ray) { return std::any_cast<const ObjectType&>(object).intersectWith(ray); };
        this->getMaterialFunction = [](const auto& object) { return std::any_cast<const ObjectType&>(object).getMaterial(); };
    }

    std::optional<std::tuple<float, glm::vec3>> intersectWith(const Ray &ray) const {
        return intersectWithFunction(object, ray);
    }

    SceneMaterial getMaterial() const {
        return getMaterialFunction(object);
    }
};
