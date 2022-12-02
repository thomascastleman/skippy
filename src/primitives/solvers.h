#pragma once

#include <optional>
#include <functional>
#include <vector>
#include <glm/glm.hpp>
#include <cmath>
#include <iostream>

#include "raytracer/ray.h"
#include "utils/intersection.h"

// All of the planes in 3d represented as 3d vectors
namespace Planes {
    extern glm::vec3 X;
    extern glm::vec3 Y;
    extern glm::vec3 Z;
}

// different possible constraints for the primitive solutions
// take in a ray and solution and return whether the solution was valid
namespace Constraints {
    using Signature = auto(const Ray &ray, float t)->bool;
    using Proxy = std::function<Signature>;

    /**
     * @brief Given a plane, returns a function that takes takes a ray and a solution, and returns whether the
     * resulting intersection point is within the cube face on that plane.
     * 
     * @param plane - An vec3 object from the Planes namespace
     */
    constexpr auto Square(auto&& plane) {
        glm::vec3 axesToCheck = glm::vec3{ 1.f, 1.f, 1.f} - plane;

        return [=](const Ray& ray, float t) {
            glm::vec3 pt = ray.getPoint(t);
            glm::vec3 ptOnPlane = pt * axesToCheck;

            return t >= 0
                   && ptOnPlane.x >= -0.5 && ptOnPlane.x <= 0.5
                   && ptOnPlane.y >= -0.5 && ptOnPlane.y <= 0.5
                   && ptOnPlane.z >= -0.5 && ptOnPlane.z <= 0.5;
        };
    }

     /**
     * @brief Returns a lambda that takes in a ray and a solutions and then returns a boolean denoting whether the intersection
     * point is within the circle defined on the y plane
     */
    constexpr auto Circle() {
        return [=](const Ray& ray, float t) {
            glm::vec3 pt = ray.getPoint(t);

            return t >= 0 && powf(pt.x, 2) + powf(pt.z, 2) <= powf(0.5, 2);
        };
    }

    /**
     * @brief Returns a lambda that takes in a ray and a solutions and then returns a boolean denoting whether the intersection
     * point is within the height range of [-0.5, 0.5]
     */
    constexpr auto Height() {
        return [=](const Ray& ray, float t) {
            glm::vec3 pt = ray.getPoint(t);

            return t >= 0 && pt.y >= -0.5f && pt.y <= 0.5f;
        };
    }

    /**
     * @brief Returns a labmda that takes in a ray and a solution and just makes sure the solution isn't negative
     */
    constexpr auto None() {
        return [=](const Ray& ray, float t) {
            return t >= 0;
        };
    }
}

namespace TextureMappers {
    // Ray, dist -> (u, v)
    using Signature = auto(const Ray& ray, float t)->std::tuple<float, float>;
    using Proxy = std::function<Signature>;

    /**
     * @brief Plane - Given a plane and a position along that plane, it returns a lambda that correctly returns
     * a uv value given a ray and an intersection distance.
     */
    constexpr auto Plane(auto&& plane, auto&& pos) {
        return [=](const Ray& ray, const float t) {
            glm::vec3 pt = ray.getPoint(t);

            if (plane.x != 0.f) {
                if (pos == 0.5f) {
                    return std::tuple<float, float>{ -pt.z + 0.5f, pt.y + 0.5f};
                } else {
                    return std::tuple<float, float>{ pt.z + 0.5f, pt.y + 0.5f };
                }
            }

            else if (plane.y != 0.f) {
                if (pos == 0.5f) {
                    return std::tuple<float, float>{ pt.x + 0.5f, -pt.z + 0.5f };
                } else {
                    return std::tuple<float, float>{ pt.x + 0.5f, pt.z + 0.5f };
                }
            }

            else { // if pt.z == 0
                if (pos == 0.5f) {
                    return std::tuple<float, float>{ pt.x + 0.5f, pt.y + 0.5f };
                } else {
                    return std::tuple<float, float>{ -pt.x + 0.5f, pt.y + 0.5f };
                }
            }
        };
    }

    /**
     * @brief ConeOrCylinder - Returns a lamda that given a ray and intersection distance, returns the proper
     * uv texture value on a cone or cyinder at that point.
     */
    constexpr auto ConeOrCylinder() {
        return [=](const Ray& ray, const float t) {
            glm::vec3 pt = ray.getPoint(t);

            float u;
            float theta = atan2f(pt.z, pt.x);
            if (theta < 0) {
                u = -theta / (2 * M_PI);
            } else {
                u = 1.f - (theta / (2 * M_PI));
            }

            return std::tuple<float, float>{ u, pt.y + 0.5f };
        };
    }

    /**
     * @brief ConeOrCylinder - Returns a lamda that given a ray and intersection distance, returns the proper
     * uv texture value on a unit sphere at that point.
     */
    constexpr auto Sphere() {
        return [=](const Ray& ray, const float t) {
            glm::vec3 pt = ray.getPoint(t);

            float v = (asinf(pt.y / 0.5f) / M_PI) + 0.5f;
            float u;

            if (v == 0.f || v == 1.f) {
                u = 0.5f;
            } else {
                float theta = atan2f(pt.z, pt.x);
                if (theta < 0) {
                    u = -theta / (2 * M_PI);
                } else {
                    u = 1.f - (theta / (2 * M_PI));
                }
            }

            return std::tuple<float, float>{ u, v };
        };
    }
}


namespace Normals {
    using Signature = auto(const Ray &ray, float t)->bool;
    using Proxy = std::function<Signature>;

    /**
     * @brief Takes in some plane and returns a lambda that given a ray and a potential solutions returns the normal vector of that plane at that point.
     * 
     * @param plane - The plane the normal is coming from.
     */
    constexpr auto Plane(auto&& plane) {
        return [=](const Ray& ray, float t) {
            glm::vec3 pt = ray.getPoint(t);
            return glm::sign(pt * plane) * plane;
        };
    }

    /**
     * @brief Returns some lambda that given a ray and solution returns the cone normal at that intersection point.
     */
    constexpr auto Cone() {
        return [=](const Ray& ray, float t) {
            glm::vec3 pt = ray.getPoint(t);

            float xNorm = (2 * pt.x);
            float yNorm = -(1.f/4.f) * (2 * pt.y - 1);
            float zNorm = (2 * pt.z);

            return glm::vec3{ xNorm, yNorm, zNorm };
        };
    }

    /**
     * @brief Returns some lambda that given a ray and solution returns the cylinder normal at that intersection point.
     */
    constexpr auto Cylinder() {
        return [=](const Ray& ray, float t) {
            glm::vec3 pt = ray.getPoint(t);

            float xNorm = (2 * pt.x);
            float yNorm = 0;
            float zNorm = (2 * pt.z);

            return glm::vec3{ xNorm, yNorm, zNorm };
        };
    }

    /**
     * @brief Returns some lambda that given a ray and solution returns the sphere normal at that intersection point.
     */
    constexpr auto Sphere() {
        return [=](const Ray& ray, float t) {
            glm::vec3 pt = ray.getPoint(t);

            float xNorm = (2 * pt.x);
            float yNorm = (2 * pt.y);
            float zNorm = (2 * pt.z);

            return glm::vec3{ xNorm, yNorm, zNorm };
        };
    }
}

namespace Solvers {
    using Signature = auto(const Ray &ray)->std::vector<std::tuple<float, glm::vec3, std::tuple<float, float>>>;
    using Proxy = std::function<Signature>;

    /**
     * @brief Given a plane, a position along the axis nor in the plane, and a constraint, it returns a lambda that takes in a 
     * ray and returns all valid intersections along that ray with the plane in a vector.
     * 
     * @param plane - A plane as defined in the namespace Planes
     * @param pos - A position along the axis not in the plane
     * @param constraint - A constraint implicit function of whether a solution is valid in the plane
     */
    constexpr auto Plane(auto&& plane, float pos, auto&& constraint, auto&& mapper) {
        auto normal = Normals::Plane(plane);

        // calculate the t value differently for each plane
        auto getT = [=](const Ray& ray) {
            if (plane.x == 1) {
                return (pos - ray.getPos().x) / ray.getDir().x;
            } else if (plane.y == 1) {
                return (pos - ray.getPos().y) / ray.getDir().y;
            } else {
                return (pos - ray.getPos().z) / ray.getDir().z;
            }
        };

        return [=](const Ray& ray) {
            float t = getT(ray);

            std::vector<Intersection::Intersection> solutions;
            if (constraint(ray, t)) {
                solutions.push_back(Intersection::Intersection{ t, normal(ray, t), mapper(ray, t)});
            }

            return solutions;
        };
    }

    /**
     * @brief Given some a, b, and c coefficients, a constraint implicit function and a normal implicit function
     * it returns a lambda that given a ray will return all valid intersections as solved by the inputted quadratic.
     * 
     * @param a - coefficient for the quadratic
     * @param b - coefficient for the quadratic
     * @param c - coefficient for the quadratic
     * @param constraint - The constraint for the resulting solution to define a valid intersection
     * @param normal - An implicit function that can calculate the resulting normal of an intersection point.
     */
    constexpr auto Quadratic(float a, float b, float c, auto&& constraint, auto&& normal, auto&& mapper) {
        return [=](const Ray& ray) {
            std::vector<Intersection::Intersection> solutions;

            float descriminant = powf(b, 2) - (4 * a * c);

            if (descriminant >= 0.f) {
                float t = (-b + sqrt(descriminant)) / (2 * a);
                if (constraint(ray, t)) {
                    solutions.push_back(Intersection::Intersection{ t, normal(ray, t), mapper(ray, t) });
                }
            }

            if (descriminant > 0.f) {
                float t = (-b - sqrt(descriminant)) / (2 * a);
                if (constraint(ray, t)) {
                    solutions.push_back(Intersection::Intersection{ t, normal(ray, t), mapper(ray, t) });
                }
            }

            return solutions;
        };
    }

    /**
     * @brief Returns a lamda that finds intersections in a square on a plane with size 1
     * 
     * @param plane - The plane of intersection
     * @param pos - The position of the plane on its 3rd axis 
     */
    constexpr auto Square(auto&& plane, float pos) {
        return Plane(plane, pos, Constraints::Square(plane), TextureMappers::Plane(plane, pos));
    }

    /**
     * @brief Returns a lamda that finds intersections in a circle on a plane with size 1
     * 
     * @param plane - The plane of intersection
     * @param pos - The position of the plane on its 3rd axis 
     */
    constexpr auto Circle(float pos) {
        return Plane(Planes::Y, pos, Constraints::Circle(), TextureMappers::Plane(Planes::Y, pos));
    }

    /**
     * @brief Returns a lamda that finds intersections on a cone body
     */
    constexpr auto Cone() {
        return [=](const Ray& ray) {
            const glm::vec3& p = ray.getPos();
            const glm::vec3& d = ray.getDir();

            float a = powf(d.x, 2) + powf(d.z, 2) - (1.f/4.f) * powf(d.y, 2);
            float b = (2 * p.x * d.x) + (2 * p.z * d.z) + ((1.f/4.f) * d.y) - ((1.f/2.f) * p.y * d.y);
            float c = powf(p.x, 2) + powf(p.z, 2) + ((1.f/ 4.f) * p.y) - ((1.f/4.f) * powf(p.y, 2)) - (1.f/16.f);

            return Quadratic(a, b, c, Constraints::Height(), Normals::Cone(), TextureMappers::ConeOrCylinder())(ray);
        };
    }

    /**
     * @brief Returns a lamda that finds intersections on a cylinder body
     */
    constexpr auto Cylinder() {
        return [=](const Ray& ray) {
            const glm::vec3& p = ray.getPos();
            const glm::vec3& d = ray.getDir();

            float a = powf(d.x, 2) + powf(d.z, 2);
            float b = 2 * (p.x * d.x + p.z * d.z);
            float c = powf(p.x, 2) + powf(p.z, 2) - powf(1.f/2.f, 2);

            return Quadratic(a, b, c, Constraints::Height(), Normals::Cylinder(), TextureMappers::ConeOrCylinder())(ray);
        };
    }

    /**
     * @brief Returns a lamda that finds intersections on a sphere body
     */
    constexpr auto Sphere() {
        return [=](const Ray& ray) {
            const glm::vec3& p = ray.getPos();
            const glm::vec3& d = ray.getDir();

            float a = powf(d.x, 2) + powf(d.y, 2) + powf(d.z, 2);
            float b = 2 * (p.x * d.x + p.y * d.y + p.z * d.z);
            float c = powf(p.x, 2) + powf(p.y, 2) + powf(p.z, 2) - powf(1.f/2.f, 2);

            return Quadratic(a, b, c, Constraints::None(), Normals::Sphere(), TextureMappers::Sphere())(ray);
        };
    }

    // vectors contaning all solvers necessary to get all intersections with a given shape
    extern std::vector<Solvers::Proxy> CubeSolvers;
    extern std::vector<Solvers::Proxy> ConeSolvers;
    extern std::vector<Solvers::Proxy> CylinderSolvers;
    extern std::vector<Solvers::Proxy> SphereSolvers;
}

