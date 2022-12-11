#include "sceneparser.h"
#include "scenefilereader.h"
#include "glm/gtx/transform.hpp"

#include <chrono>
#include <memory>
#include <iostream>

/**
 * @brief SceneParser::buildTransformMatrix - Takes in a SceneTransformation object and returns the 4d matrix representing that transform
 * @param transformation - A scene transformation object
 * @return A matric representing that transformation
 */
glm::mat4 SceneParser::buildTransformMatrix(InterpolatedSceneTransformation* transformation, int frame) {
   switch (transformation->type) {
        case TransformationType::TRANSFORMATION_TRANSLATE:
            return glm::translate(transformation->translate(frame));
        case TransformationType::TRANSFORMATION_SCALE:
            return glm::scale(transformation->scale(frame));
        case TransformationType::TRANSFORMATION_ROTATE:
            return glm::rotate(transformation->angle(frame), transformation->rotate(frame));
        default:
            std::cout << "ERROR: invalid transformation type" << std::endl;
            exit(1);
   }
}

SceneLightData SceneParser::buildLight(InterpolatedSceneLightData *light, glm::mat4 &ctm, int frame) {
    std::cout << "Building light" << std::endl;

    SceneLightData lightAtFrame;
    lightAtFrame.id = light->id;
    lightAtFrame.type = light->type;

    lightAtFrame.color = light->color(frame);

    switch (light->type) {
        case LightType::LIGHT_DIRECTIONAL:
            lightAtFrame.dir = light->dir(frame);
            break;
        case LightType::LIGHT_POINT:
            lightAtFrame.pos = ctm * glm::vec4{ 0.f, 0.f, 0.f, 1.f };
            lightAtFrame.function = light->function(frame);
            break;
        case LightType::LIGHT_SPOT:
            lightAtFrame.pos = ctm * glm::vec4{ 0.f, 0.f, 0.f, 1.f };
            lightAtFrame.function = light->function(frame);
            lightAtFrame.dir = light->dir(frame);
            lightAtFrame.angle = light->angle(frame);
            lightAtFrame.penumbra = light->penumbra(frame);
            break;
        default:
            std::cout << "ERROR: cannot build light of type: " << unsigned(light->type) << std::endl;
            exit(1);
    }

    return lightAtFrame;
}

SceneCameraData SceneParser::buildCamera(InterpolatedCameraData* camera, glm::mat4 &ctm, int frame) {
    std::cout << "Building camera" << std::endl;

    SceneCameraData cameraAtFrame;

    cameraAtFrame.heightAngle = camera->heightAngle(frame);

    cameraAtFrame.pos = ctm * glm::vec4{ 0.f, 0.f, 0.f, 1.f };
    cameraAtFrame.look = camera->look(frame);
    std::cout << "look: " << cameraAtFrame.look.x << " " << cameraAtFrame.look.y << " " << cameraAtFrame.look.z << std::endl;

    std::cout << "Height up" << std::endl;
    cameraAtFrame.up = camera->up(frame);

    return cameraAtFrame;
}

/**
 * @brief SceneParser::buildRenderShapes - Traverses scene graph, while populating a vector of all shapes and their cumulative transformation matricies
 * @param node - A pointer to a node in the graph (originally called on root)
 * @param shapes - A vector of shape data to be populated
 * @param ctm - The cumulative transformation matrix to this poitn
 */
void SceneParser::buildRenderObjects(SceneNode *node, RenderData *rd, glm::mat4 ctm, int frame) {
    // multiply all the transforms on this node together
    glm::mat4 nodeTransform(1.0f); // start as identity matrixx
    for (auto *transform : node->transformations) {
        nodeTransform = nodeTransform * buildTransformMatrix(transform, frame);
    }

    // update the cumulative transformation object
    ctm = ctm * nodeTransform;

    // if this is a leaf node, add the shapes
    if (node->primitives.size() > 0) {
        for (auto *primitive : node->primitives) {
            rd->shapes.push_back({ *primitive,  ctm });
        }
        return;
    }

    // if this is a leaf node, add the lights
    if (node->lights.size() > 0) {
        for (auto *light : node->lights) {
            rd->lights.push_back(buildLight(light, ctm, frame));
        }
        return;
    }

    // if this is a leaf node, construct the camera
    if (node->camera.has_value()) {
        rd->cameraData = buildCamera(node->camera.value(), ctm, frame);
    }

    // recur on each child node
    for (auto *child : node->children) {
        buildRenderObjects(child, rd, ctm, frame);
    }

    return;
}

/**
 * @brief Given a filepath and some render data, parse the scene file into the render Data object
 * 
 * @param filepath - path to an xml scene file
 * @param renderData - some renderData object to populate
 * @return true - success
 * @return false - failure
 */
bool SceneParser::parse(std::string filepath, std::vector<RenderData*> &renderData) {
    ScenefileReader fileReader = ScenefileReader(filepath);
    bool success = fileReader.readXML();
    std::cout << "Parsed file" << std::endl;

    if (!success) {
        return false;
    }

    SceneGlobalData globalData = fileReader.getGlobalData();
    for (int i = 0; i < globalData.numFrames; i++) {
        RenderData* rd = new RenderData();

        rd->globalData = fileReader.getGlobalData();

        // make sure shape data is cleared
        rd->shapes.clear();
        rd->lights.clear();

        // start at the root with an identity matrix
        buildRenderObjects(fileReader.getRootNode(), rd, glm::mat4(1.0f), i);

        renderData.push_back(rd);
    }

    return true;
}

