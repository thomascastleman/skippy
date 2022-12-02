#pragma once

#include "camera/camera.h"
#include "utils/scenedata.h"
#include "utils/sceneparser.h"
//#include "primitives/shape.h"
#include "primitives/worldprimitive.h"
#include "lighting/lights.h"
#include <map>
#include "utils/rgba.h"
#include "texture/texture.h"

// A class representing a scene to be ray-traced

// Feel free to make your own design choices for RayTraceScene, the functions below are all optional / for your convenience.
// You can either implement and use these getters, or make your own design.
// If you decide to make your own design, feel free to delete these as TAs won't rely on them to grade your assignments.

class RayTraceScene
{
public:
    RayTraceScene(int width, int height, const RenderData &metaData);

    // The getter of the width of the scene
    const int& width() const;

    // The getter of the height of the scene
    const int& height() const;

    // The getter of the global data of the scene
    const SceneGlobalData& getGlobalData() const;

    // The getter of the shared pointer to the camera instance of the scene
    const Camera& getCamera() const;

//    const std::vector<Shape>& getShapeData() const;
    const std::vector<WorldPrimitive::Proxy>& getPrims() const;
    const std::vector<Lights::Proxy>& getLights() const;
    const std::map<std::string, Texture::Texture>& getTextures() const;

private:
//    static const std::vector<Shape> buildShapes(const std::vector<RenderShapeData>& renderShapes);
    void buildPrims(const std::vector<RenderShapeData>& renderShapes);
    void buildLights(const std::vector<SceneLightData>& sceneLights);

    const int m_canvasWidth;
    const int m_canvasHeight;
    const RenderData& m_data;
    const Camera m_camera;

//    const std::vector<Shape> m_shapes;
    std::vector<WorldPrimitive::Proxy> m_prims;
    std::vector<Lights::Proxy> m_lights;
    std::map<std::string, Texture::Texture> m_textures;
};
