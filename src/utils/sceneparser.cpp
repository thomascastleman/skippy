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

/**
 * @brief SceneParser::buildRenderShapes - Traverses scene graph, while populating a vector of all shapes and their cumulative transformation matricies
 * @param node - A pointer to a node in the graph (originally called on root)
 * @param shapes - A vector of shape data to be populated
 * @param ctm - The cumulative transformation matrix to this poitn
 */
void SceneParser::buildRenderShapes(SceneNode *node, std::vector<RenderShapeData> &shapes, glm::mat4 ctm, int frame) {
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
            shapes.push_back({ *primitive,  ctm });
        }
        return;
    }

    // recur on each child node
    for (auto *child : node->children) {
        buildRenderShapes(child, shapes, ctm, frame);
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
    for (int i = 0; i < ceil(globalData.duration * globalData.framerate); i++) {
        RenderData* rd = new RenderData();

        rd->globalData = fileReader.getGlobalData();
        rd->lights = fileReader.getLights();
        rd->cameraData = fileReader.getCameraData();


        // make sure shape data is cleared
        rd->shapes.clear();

        // start at the root with an identity matrix
        buildRenderShapes(fileReader.getRootNode(), rd->shapes, glm::mat4(1.0f), i);

        renderData.push_back(rd);
    }



    return true;
}

