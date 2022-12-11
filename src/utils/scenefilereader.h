#pragma once

#include "scenedata.h"

#include <vector>
#include <map>

#include <QDomDocument>

// This class parses the scene graph specified by the CS123 Xml file format.
class ScenefileReader {
public:
    // Create a ScenefileReader, passing it the scene file.
    ScenefileReader(const std::string& filename);

    // Clean up all data for the scene
    ~ScenefileReader();

    // Parse the XML scene file. Returns false if scene is invalid.
    bool readXML();

    SceneGlobalData getGlobalData() const;

    SceneNode* getRootNode() const;

private:
    // The filename should be contained within this parser implementation.
    // If you want to parse a new file, instantiate a different parser.
    bool parseGlobalData(const QDomElement &globaldata);
    bool parseObjectData(const QDomElement &object);

    float linearInterpolate(int frame, int startingFrame, int endingFrame, float start, float end);
    std::function<glm::vec3 (int)> interpolateVec3(std::vector<std::tuple<int, glm::vec3> > &keyFrameData);
    std::function<glm::vec4 (int)> interpolateVec4(std::vector<std::tuple<int, glm::vec4> > &keyFrameData);
    std::function<float (int)> interpolateFloat(std::vector<std::tuple<int, float> > &keyFrameData);

    void interpolateLight(SceneLightKeyFrameData &lightKeyFrameData, InterpolatedSceneLightData *interpolatedSceneLightData);
    void interpolateCamera(SceneCameraKeyFrameData &cameraKeyFrameData, InterpolatedCameraData *camera);

    void interpolateTranslation(std::vector<std::tuple<int, SceneTransformation *> > &translations, SceneNode *node);
    void interpolateScale(std::vector<std::tuple<int, SceneTransformation *> > &scales, SceneNode *node);
    void interpolateRotation(std::vector<std::tuple<int, SceneTransformation *> > &rotations, SceneNode *node);

    bool parseKeyFrame(const QDomElement &keyFrame, TransformationMap& tm, std::vector<std::string>& order);
    bool parseLightKeyFrame(const QDomElement &keyFrame, SceneLightKeyFrameData &lightKeyFrameData);
    bool parseCameraKeyFrame(const QDomElement &keyFrame, SceneCameraKeyFrameData &cameraKeyFrameData);

    bool parseTransBlock(const QDomElement &transblock, SceneNode* node);
    bool parseCameraData(const QDomElement &cameradata, SceneNode* node);
    bool parseLightData(const QDomElement &lightdata, SceneNode *node);
    bool parsePrimitive(const QDomElement &prim, SceneNode* node);

    std::string file_name;
    mutable std::map<std::string, SceneNode*> m_objects;

    SceneGlobalData m_globalData;
    std::vector<SceneNode*> m_nodes;

    bool foundCamera = false;
};
