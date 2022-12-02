#include "solvers.h"

// The three planes for the cube solvers
glm::vec3 Planes::X = glm::vec3{ 1.f, 0.f, 0.f };
glm::vec3 Planes::Y = glm::vec3{ 0.f, 1.f, 0.f };
glm::vec3 Planes::Z = glm::vec3{ 0.f, 0.f, 1.f };

// The cube solvers need a square solver for each face of the cube
std::vector<Solvers::Proxy> Solvers::CubeSolvers = std::vector<Solvers::Proxy> {
    Solvers::Square(Planes::Z, 0.5f), // front plane
    Solvers::Square(Planes::Z, -0.5f), // back plane
    Solvers::Square(Planes::Y, 0.5f), // top plane
    Solvers::Square(Planes::Y, -0.5f), // bottom plane
    Solvers::Square(Planes::X, 0.5f), // left plane
    Solvers::Square(Planes::X, -0.5f), // right plane
};

// The cone solvers need one solver for the body and one solver for the cap
std::vector<Solvers::Proxy> Solvers::ConeSolvers = std::vector<Solvers::Proxy> {
    Solvers::Cone(),
    Solvers::Circle(-0.5f)
};

// The cylinder solvers need one solver for the body and two solvers for the caps
std::vector<Solvers::Proxy> Solvers::CylinderSolvers = std::vector<Solvers::Proxy> {
    Solvers::Cylinder(),
    Solvers::Circle(-0.5f),
    Solvers::Circle(0.5f)
};

// The sphere just needs a single solver for the body
std::vector<Solvers::Proxy> Solvers::SphereSolvers = std::vector<Solvers::Proxy> {
    Solvers::Sphere()
};

