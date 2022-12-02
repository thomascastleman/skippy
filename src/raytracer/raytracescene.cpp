#include "raytracescene.h"

#include "primitives/objectprimitives.h"
#include "texture/texture.h"

/**
 * @brief Builds all the shape primitives based off of RenderShapeData
 * 
 * @param renderShapes - A reference to all the shape data
 */
void RayTraceScene::buildPrims(const std::vector<RenderShapeData> &renderShapes) {
    for (const RenderShapeData &renderShape : renderShapes) {
        const SceneMaterial& mat = renderShape.primitive.material;
        if (mat.textureMap.isUsed && !m_textures.contains(mat.textureMap.filename)) {
            Texture::load(mat.textureMap.filename, m_textures);
        }

        switch (renderShape.primitive.type) {
            case PrimitiveType::PRIMITIVE_CUBE:
                m_prims.push_back(WorldPrimitive::Primitive(ObjectPrimitives::Cube(), renderShape.ctm, mat));
                break;
            case PrimitiveType::PRIMITIVE_CONE:
                m_prims.push_back(WorldPrimitive::Primitive(ObjectPrimitives::Cone(), renderShape.ctm, mat));
                break;
             case PrimitiveType::PRIMITIVE_CYLINDER:
                m_prims.push_back(WorldPrimitive::Primitive(ObjectPrimitives::Cylinder(), renderShape.ctm, mat));
                break;
             case PrimitiveType::PRIMITIVE_SPHERE:
                m_prims.push_back(WorldPrimitive::Primitive(ObjectPrimitives::Sphere(), renderShape.ctm, mat));
                break;
             default:
                break;
        }
    }
}

/**
 * @brief Builds all the scene lights based off of SceneLightData
 * 
 * @param sceneLights = A reference to all the raw light data
 */
void RayTraceScene::buildLights(const std::vector<SceneLightData> &sceneLights) {
    for (const SceneLightData &sceneLight : sceneLights) {
       switch (sceneLight.type) {
            case LightType::LIGHT_DIRECTIONAL:
                m_lights.push_back(Lights::Directional(sceneLight.dir, sceneLight.color));
                break;
            case LightType::LIGHT_POINT:
                m_lights.push_back(Lights::Point(sceneLight.pos, sceneLight.function, sceneLight.color));
                break;
            case LightType::LIGHT_SPOT:
                m_lights.push_back(Lights::Spot(sceneLight.pos, sceneLight.dir, sceneLight.angle, sceneLight.penumbra, sceneLight.function, sceneLight.color));
                break;
             default:
                break;
       }
    }
}

RayTraceScene::RayTraceScene(int width, int height, const RenderData &metaData):
    m_data(metaData),
    m_canvasWidth(width),
    m_canvasHeight(height),
    m_camera(metaData.cameraData, width / float(height))
{
    buildPrims(metaData.shapes);
    buildLights(metaData.lights);
}

/**
 * @brief Get the canvas width of the scene
 * 
 * @return const int& 
 */
const int& RayTraceScene::width() const {
    return m_canvasWidth;
}

/**
 * @brief Get the canvas height of the scene
 * 
 * @return const int& 
 */
const int& RayTraceScene::height() const {
    return m_canvasHeight;
}

/**
 * @brief Get a reference to all the global data in the scene
 * 
 * @return const SceneGlobalData& 
 */
const SceneGlobalData& RayTraceScene::getGlobalData() const {
    return m_data.globalData;
}

/**
 * @brief Get a reference to the scene's camera object
 * 
 * @return const Camera& 
 */
const Camera& RayTraceScene::getCamera() const {
    return m_camera;
}


/**
 * @brief Get a reference to all the shape primitive implicit functions in a vector
 * 
 * @return const std::vector<WorldPrimitive::Proxy>& 
 */
const std::vector<WorldPrimitive::Proxy>& RayTraceScene::getPrims() const {
    return m_prims;
}

/**
 * @brief Get a reference to all the light implicit functions in a vector
 * 
 * @return const std::vector<Lights::Proxy>& 
 */
const std::vector<Lights::Proxy>& RayTraceScene::getLights() const {
    return m_lights;
}


const std::map<std::string, Texture::Texture>& RayTraceScene::getTextures() const {
    return m_textures;
}
