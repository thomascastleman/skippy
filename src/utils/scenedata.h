#pragma once

#include <vector>
#include <string>
#include <functional>
#include <map>
#include <optional>

#include <glm/glm.hpp>

// Enum of the types of virtual lights that might be in the scene
enum class LightType {
    LIGHT_POINT,
    LIGHT_DIRECTIONAL,
    LIGHT_SPOT,
    LIGHT_AREA // No longer supported
};

// Enum of the types of primitives that might be in the scene
enum class PrimitiveType {
    PRIMITIVE_CUBE,
    PRIMITIVE_CONE,
    PRIMITIVE_CYLINDER,
    PRIMITIVE_TORUS,
    PRIMITIVE_SPHERE,
    PRIMITIVE_MESH
};

// Enum of the types of transformations that can be applied
enum class TransformationType {
    TRANSFORMATION_TRANSLATE,
    TRANSFORMATION_SCALE,
    TRANSFORMATION_ROTATE,
    TRANSFORMATION_MATRIX
};

// Type which can be used to store an RGBA color in floats [0,1]
using SceneColor = glm::vec4;

// Struct which contains the global color coefficients of a scene.
// These are multiplied with the object-specific materials in the lighting equation.
struct SceneGlobalData  {
    float ka; // Ambient term
    float kd; // Diffuse term
    float ks; // Specular term
    float kt; // Transparency; used for extra credit (refraction)

    int framerate; // The frames per second of the output movie
    float duration; // The duration of the output movie in seconds
    int numFrames;
};

// Struct which contains data for a single light
struct SceneLightData {
    int id;
    LightType type;

    SceneColor color;
    glm::vec3 function;  // Attenuation function

    glm::vec4 pos;       // Not applicable to directional lights
    glm::vec4 dir;       // Not applicable to point lights

    float penumbra;      // Only applicable to spot lights, in RADIANS
    float angle;         // Only applicable to spot lights, in RADIANS

    float width, height; // No longer supported (area lights)
};

// stores data for a light across key frames
struct SceneLightKeyFrameData {
    int id;
    LightType type;

    std::vector<std::tuple<int, SceneColor>> color;
    std::vector<std::tuple<int, glm::vec3>> function;
    std::vector<std::tuple<int, glm::vec4>> dir;
    std::vector<std::tuple<int, float>> penumbra;
    std::vector<std::tuple<int, float>> angle;
};

// Light capable of interpolating between most of its parameter values
struct InterpolatedSceneLightData {
    int id;
    LightType type;

    std::function<SceneColor(int)> color;
    std::function<glm::vec3(int)> function;

    std::function<glm::vec4(int)> dir;

    std::function<float(int)> penumbra;
    std::function<float(int)> angle;
};

// Struct which contains data for the camera of a scene
struct SceneCameraData {
    glm::vec4 pos;
    glm::vec4 look;
    glm::vec4 up;

    float heightAngle;   // The height angle of the camera in RADIANS

    float aperture;      // Only applicable for depth of field
    float focalLength;   // Only applicable for depth of field
};

// Camera which can interpolate between its parameters.
struct InterpolatedCameraData {
    std::function<glm::vec4(int)> look;
    std::function<glm::vec4(int)> up;
    std::function<float(int)> heightAngle;
};

// stores data for a camera accross multiple keyframes
struct SceneCameraKeyFrameData {
    std::vector<std::tuple<int, glm::vec4>> look;
    std::vector<std::tuple<int, glm::vec4>> up;
    std::vector<std::tuple<int, float>> heightAngle;
};

// Struct which contains data for texture mapping files
struct SceneFileMap {
    SceneFileMap() : isUsed(false) {}

    bool isUsed;
    std::string filename;

    float repeatU;
    float repeatV;

    void clear() {
       isUsed = false;
       repeatU = 0.0f;
       repeatV = 0.0f;
       filename = std::string();
    }
};

// Struct which contains data for a material (e.g. one which might be assigned to an object)
struct SceneMaterial {
   SceneColor cAmbient;     // Ambient term
   SceneColor cDiffuse;     // Diffuse term
   SceneColor cSpecular;    // Specular term
   float shininess;         // Specular exponent

   SceneColor cReflective;  // Used to weight contribution of reflected ray lighting (via multiplication)

   SceneColor cTransparent; // Transparency;        used for extra credit (refraction)
   float ior;               // Index of refraction; used for extra credit (refraction)

   SceneFileMap textureMap; // Used for texture mapping
   float blend;             // Used for texture mapping

   SceneColor cEmissive;    // Not used
   SceneFileMap bumpMap;    // Not used

   void clear() {
       cAmbient    = glm::vec4(0);
       cDiffuse    = glm::vec4(0);
       cSpecular   = glm::vec4(0);
       shininess   = 0;

       cReflective = glm::vec4(0);

       cTransparent = glm::vec4(0);
       ior = 0;

       textureMap.clear();
       blend = 0;

       cEmissive = glm::vec4(0);
       bumpMap.clear();
   }
};

// Struct which contains data for a single primitive in a scene
struct ScenePrimitive {
   PrimitiveType type;
   SceneMaterial material;
   std::string   meshfile; // Used for triangle meshes
};

// Struct which contains data for a transformation.
struct SceneTransformation {
    TransformationType type;

    glm::vec3 translate;
    glm::vec3 scale;
    glm::vec3 rotate;
    float angle;
};

using TransformationMap = std::map<std::string, std::vector<std::tuple<int, SceneTransformation*>>>;

// Struct which contains data for a trnasformation over time
struct InterpolatedSceneTransformation {
    TransformationType type;

    std::function<glm::vec3(int)> translate;
    std::function<glm::vec3(int)> scale;
    std::function<glm::vec3(int)> rotate;    // Only applicable when rotating.    Defines the axis of rotation; should be a unit vector.
    std::function<float(int)> angle;         // Only applicable when rotating.    Defines the angle to rotate by in RADIANS, following the right-hand rule.

    glm::mat4 matrix;    // Only applicable when transforming by a custom matrix. This is that custom matrix.
};

// Struct which represents a node in the scene graph/tree, to be parsed by the student's `SceneParser`.
struct SceneNode {
   std::vector<InterpolatedSceneTransformation*> transformations; // Note the order of transformations described in lab 5
   std::vector<ScenePrimitive*>      			 primitives;
   std::vector<InterpolatedSceneLightData*> 	 lights;
   std::vector<SceneNode*>           			 children;
   std::optional<InterpolatedCameraData*>		 camera;
};
