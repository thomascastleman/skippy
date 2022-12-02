#pragma once

#include <glm/glm.hpp>
#include "utils/rgba.h"
#include "raytracescene.h"

// A class representing a ray-tracer

class RayTracer
{
public:
    struct Config {
        bool enableShadow        = false;
        bool enableReflection    = false;
        bool enableRefraction    = false;
        bool enableTextureMap    = false;
        bool enableTextureFilter = false;
        bool enableParallelism   = false;
        bool enableSuperSample   = false;
        int  numSamples          =     0;
        bool enablePostProcess   = false;
        bool enableAcceleration  = false;
        bool enableDepthOfField  = false;
    };

public:
    RayTracer(Config config);

    // Renders the scene synchronously.
    // The ray-tracer will render the scene and fill imageData in-place.
    // @param imageData The pointer to the imageData to be filled.
    // @param scene The scene to be rendered.
    void render(RGBA *imageData, const RayTraceScene &scene);

    glm::vec4 traceRay(const Ray &ray, const RayTraceScene &scene, const int depth = 0);

private:
    const Config m_config;
};

