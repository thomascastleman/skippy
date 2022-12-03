#include "scenefilereader.h"
#include "scenedata.h"

#include "glm/gtc/type_ptr.hpp"

#include <cassert>
#include <cstring>
#include <iostream>
#include <filesystem>

#include <QFile>

#define ERROR_AT(e) "error at line " << e.lineNumber() << " col " << e.columnNumber() << ": "
#define PARSE_ERROR(e) std::cout << ERROR_AT(e) << "could not parse <" << e.tagName().toStdString() \
   << ">" << std::endl
#define UNSUPPORTED_ELEMENT(e) std::cout << ERROR_AT(e) << "unsupported element <" \
   << e.tagName().toStdString() << ">" << std::endl;

// Students, please ignore this file.

ScenefileReader::ScenefileReader(const std::string& name)
{
   file_name = name;

   memset(&m_cameraData, 0, sizeof(SceneCameraData));
   memset(&m_globalData, 0, sizeof(SceneGlobalData));
   m_objects.clear();
   m_lights.clear();
   m_nodes.clear();
}

ScenefileReader::~ScenefileReader()
{
   std::vector<SceneLightData*>::iterator lights;
   for (lights = m_lights.begin(); lights != m_lights.end(); lights++) {
       delete *lights;
   }

   // Delete all Scene Nodes
   for (unsigned int node = 0; node < m_nodes.size(); node++) {
       for (size_t i = 0; i < (m_nodes[node])->transformations.size(); i++) {
           delete (m_nodes[node])->transformations[i];
       }
       for (size_t i = 0; i < (m_nodes[node])->primitives.size(); i++) {
           delete (m_nodes[node])->primitives[i];
       }
       (m_nodes[node])->transformations.clear();
       (m_nodes[node])->primitives.clear();
       (m_nodes[node])->children.clear();
       delete m_nodes[node];
   }

   m_nodes.clear();
   m_lights.clear();
   m_objects.clear();
}

SceneGlobalData ScenefileReader::getGlobalData() const {
   return m_globalData;
}

SceneCameraData ScenefileReader::getCameraData() const {
   return m_cameraData;
}

std::vector<SceneLightData> ScenefileReader::getLights() const {
   std::vector<SceneLightData> ret{};
   ret.reserve(m_lights.size());

   for (auto light : m_lights) {
       ret.emplace_back(*light);
   }
   return ret;
}

SceneNode* ScenefileReader::getRootNode() const {
   std::map<std::string, SceneNode*>::iterator node = m_objects.find("root");
   if (node == m_objects.end())
       return nullptr;
   return m_objects["root"];
}

// This is where it all goes down...
bool ScenefileReader::readXML() {
    std::cout << "Rading xml" << std::endl;

   // Read the file
   QFile file(file_name.c_str());
   if (!file.open(QFile::ReadOnly)) {
       std::cout << "could not open " << file_name << std::endl;
       return false;
   }

   // Load the XML document
   QDomDocument doc;
   QString errorMessage;
   int errorLine, errorColumn;
   if (!doc.setContent(&file, &errorMessage, &errorLine, &errorColumn)) {
       std::cout << "parse error at line " << errorLine << " col " << errorColumn << ": "
            << errorMessage.toStdString() << std::endl;
       return false;
   }
   file.close();

   // Get the root element
   QDomElement scenefile = doc.documentElement();
   if (scenefile.tagName() != "scenefile") {
       std::cout << "missing <scenefile>" << std::endl;
       return false;
   }

   // Default camera
   m_cameraData.pos = glm::vec4(5.f, 5.f, 5.f, 1.f);
   m_cameraData.up = glm::vec4(0.f, 1.f, 0.f, 0.f);
   m_cameraData.look = glm::vec4(-1.f, -1.f, -1.f, 0.f);
   m_cameraData.heightAngle = 45 * M_PI / 180.f;

   // Default global data
   m_globalData.ka = 0.5f;
   m_globalData.kd = 0.5f;
   m_globalData.ks = 0.5f;

   // Iterate over child elements
   QDomNode childNode = scenefile.firstChild();
   while (!childNode.isNull()) {
       QDomElement e = childNode.toElement();
       if (e.tagName() == "globaldata") {
           std::cout << "Parsing global data" << std::endl;
           if (!parseGlobalData(e))
               return false;
           std::cout << "Finished parsing global data" << std::endl;
       } else if (e.tagName() == "lightdata") {
           if (!parseLightData(e))
               return false;
       } else if (e.tagName() == "cameradata") {
           if (!parseCameraData(e))
               return false;
       } else if (e.tagName() == "object") {
           std::cout << "Parsing an object" << std::endl;
           if (!parseObjectData(e))
               return false;
       } else if (!e.isNull()) {
           UNSUPPORTED_ELEMENT(e);
           return false;
       }
       childNode = childNode.nextSibling();
   }

   std::cout << "Finished reading " << file_name << std::endl;
   return true;
}

/**
* Helper function to parse a single value, the name of which is stored in
* name.  For example, to parse <length v="0"/>, name would need to be "v".
*/
bool parseInt(const QDomElement &single, int &a, const char *name) {
   if (!single.hasAttribute(name))
       return false;
   a = single.attribute(name).toInt();
   return true;
}

/**
* Helper function to parse a single value, the name of which is stored in
* name.  For example, to parse <length v="0"/>, name would need to be "v".
*/
template <typename T> bool parseSingle(const QDomElement &single, T &a, const QString &str) {
   if (!single.hasAttribute(str))
       return false;
   a = single.attribute(str).toDouble();
   return true;
}

/**
* Helper function to parse a triple.  Each attribute is assumed to take a
* letter, which are stored in chars in order.  For example, to parse
* <pos x="0" y="0" z="0"/>, chars would need to be "xyz".
*/
template <typename T> bool parseTriple(
       const QDomElement &triple,
       T &a,
       T &b,
       T &c,
       const QString &str_a,
       const QString &str_b,
       const QString &str_c) {
   if (!triple.hasAttribute(str_a) ||
       !triple.hasAttribute(str_b) ||
       !triple.hasAttribute(str_c))
       return false;
   a = triple.attribute(str_a).toDouble();
   b = triple.attribute(str_b).toDouble();
   c = triple.attribute(str_c).toDouble();
   return true;
}

/**
* Helper function to parse a quadruple.  Each attribute is assumed to take a
* letter, which are stored in chars in order.  For example, to parse
* <color r="0" g="0" b="0" a="0"/>, chars would need to be "rgba".
*/
template <typename T> bool parseQuadruple(
       const QDomElement &quadruple,
       T &a,
       T &b,
       T &c,
       T &d,
       const QString &str_a,
       const QString &str_b,
       const QString &str_c,
       const QString &str_d) {
   if (!quadruple.hasAttribute(str_a) ||
       !quadruple.hasAttribute(str_b) ||
       !quadruple.hasAttribute(str_c) ||
       !quadruple.hasAttribute(str_d))
       return false;
   a = quadruple.attribute(str_a).toDouble();
   b = quadruple.attribute(str_b).toDouble();
   c = quadruple.attribute(str_c).toDouble();
   d = quadruple.attribute(str_d).toDouble();
   return true;
}


template <typename T> bool parseQuadruple(
       const QDomElement &quadruple,
       std::string &a,
       T &b,
       T &c,
       T &d,
       const QString &str_a,
       const QString &str_b,
       const QString &str_c,
       const QString &str_d) {
   if (!quadruple.hasAttribute(str_a) ||
       !quadruple.hasAttribute(str_b) ||
       !quadruple.hasAttribute(str_c) ||
       !quadruple.hasAttribute(str_d))
       return false;
   a = quadruple.attribute(str_a).toStdString();
   b = quadruple.attribute(str_b).toDouble();
   c = quadruple.attribute(str_c).toDouble();
   d = quadruple.attribute(str_d).toDouble();
   return true;
}

template <typename T> bool parseQuintuple(
       const QDomElement &quadruple,
       std::string &a,
       T &b,
       T &c,
       T &d,
       T &e,
       const QString &str_a,
       const QString &str_b,
       const QString &str_c,
       const QString &str_d,
       const QString &str_e) {
   if (!quadruple.hasAttribute(str_a) ||
       !quadruple.hasAttribute(str_b) ||
       !quadruple.hasAttribute(str_c) ||
       !quadruple.hasAttribute(str_d) ||
       !quadruple.hasAttribute(str_e))
       return false;
   a = quadruple.attribute(str_a).toStdString();
   b = quadruple.attribute(str_b).toDouble();
   c = quadruple.attribute(str_c).toDouble();
   d = quadruple.attribute(str_d).toDouble();
   e = quadruple.attribute(str_e).toDouble();
   return true;
}


/**
* Helper function to parse a matrix. Assumes the input matrix is row-major, which is converted to
* a column-major glm matrix.
*
* Example matrix:
*
* <matrix>
*   <row a="1" b="0" c="0" d="0"/>
*   <row a="0" b="1" c="0" d="0"/>
*   <row a="0" b="0" c="1" d="0"/>
*   <row a="0" b="0" c="0" d="1"/>
* </matrix>
*/
bool parseMatrix(const QDomElement &matrix, glm::mat4 &m) {
   float *valuePtr = glm::value_ptr(m);
   QDomNode childNode = matrix.firstChild();
   int col = 0;

   while (!childNode.isNull()) {
       QDomElement e = childNode.toElement();
       if (e.isElement()) {
           float a, b, c, d;
           if (!parseQuadruple(e, a, b, c, d, "a", "b", "c", "d")
                   && !parseQuadruple(e, a, b, c, d, "v1", "v2", "v3", "v4")) {
               PARSE_ERROR(e);
               return false;
           }
           valuePtr[0*4 + col] = a;
           valuePtr[1*4 + col] = b;
           valuePtr[2*4 + col] = c;
           valuePtr[3*4 + col] = d;
           if (++col == 4) break;
       }
       childNode = childNode.nextSibling();
   }

   return (col == 4);
}

/**
* Helper function to parse a color.  Will parse an element with r, g, b, and
* a attributes (the a attribute is optional and defaults to 1).
*/
bool parseColor(const QDomElement &color, SceneColor &c) {
   c.a = 1;
   return parseQuadruple(color, c.r, c.g, c.b, c.a, "r", "g", "b", "a") ||
          parseQuadruple(color, c.r, c.g, c.b, c.a, "x", "y", "z", "w") ||
          parseTriple(color, c.r, c.g, c.b, "r", "g", "b") ||
          parseTriple(color, c.r, c.g, c.b, "x", "y", "z");
}

/**
* Helper function to parse a texture map tag. The texture map image should be relative to the root directory of
* scenefile root. Example texture map tag:
* <texture file="/image/andyVanDam.jpg" u="1" v="1"/>
*/
bool parseMap(const QDomElement &e, SceneFileMap &map, const std::filesystem::path &basepath) {
   if (!e.hasAttribute("file"))
       return false;

   std::filesystem::path fileRelativePath(e.attribute("file").toStdString());

   map.filename = (basepath / fileRelativePath).string();
   map.repeatU = e.hasAttribute("u") ? e.attribute("u").toFloat() : 1;
   map.repeatV = e.hasAttribute("v") ? e.attribute("v").toFloat() : 1;
   map.isUsed = true;
   return true;
}

/**
* Parse a <globaldata> tag and fill in m_globalData.
*/
bool ScenefileReader::parseGlobalData(const QDomElement &globaldata) {
   // Iterate over child elements
   QDomNode childNode = globaldata.firstChild();
   while (!childNode.isNull()) {
       QDomElement e = childNode.toElement();
       if (e.tagName() == "ambientcoeff") {
           if (!parseSingle(e, m_globalData.ka, "v")) {
               PARSE_ERROR(e);
               return false;
           }
       } else if (e.tagName() == "diffusecoeff") {
           if (!parseSingle(e, m_globalData.kd, "v")) {
               PARSE_ERROR(e);
               return false;
           }
       } else if (e.tagName() == "specularcoeff") {
           if (!parseSingle(e, m_globalData.ks, "v")) {
               PARSE_ERROR(e);
               return false;
           }
       } else if (e.tagName() == "transparentcoeff") {
           if (!parseSingle(e, m_globalData.kt, "v")) {
               PARSE_ERROR(e);
               return false;
           }
       } else if (e.tagName() == "framerate") {
           if (!parseSingle(e, m_globalData.framerate, "fps")) {
               PARSE_ERROR(e);
               return false;
           }
       } else if (e.tagName() == "duration") {
           if (!parseSingle(e, m_globalData.duration, "seconds")) {
               PARSE_ERROR(e);
               return false;
           }
       }
       childNode = childNode.nextSibling();
   }

   std::cout << m_globalData.framerate << ", " << m_globalData.duration << std::endl;

   return true;
}

/**
* Parse a <lightdata> tag and add a new CS123SceneLightData to m_lights.
*/
bool ScenefileReader::parseLightData(const QDomElement &lightdata) {
   // Create a default light
   SceneLightData* light = new SceneLightData();
   m_lights.push_back(light);
   memset(light, 0, sizeof(SceneLightData));
   light->pos = glm::vec4(3.f, 3.f, 3.f, 1.f);
   light->dir = glm::vec4(0.f, 0.f, 0.f, 0.f);
   light->color.r = light->color.g = light->color.b = 1;
   light->function = glm::vec3(1, 0, 0);

   // Iterate over child elements
   QDomNode childNode = lightdata.firstChild();
   while (!childNode.isNull()) {
       QDomElement e = childNode.toElement();
       if (e.tagName() == "id") {
           if (!parseInt(e, light->id, "v")) {
               PARSE_ERROR(e);
               return false;
           }
       } else if (e.tagName() == "type") {
           if (!e.hasAttribute("v")) {
               PARSE_ERROR(e);
               return false;
           }
           if (e.attribute("v") == "directional") light->type = LightType::LIGHT_DIRECTIONAL;
           else if (e.attribute("v") == "point") light->type = LightType::LIGHT_POINT;
           else if (e.attribute("v") == "spot") light->type = LightType::LIGHT_SPOT;
           else if (e.attribute("v") == "area") light->type = LightType::LIGHT_AREA;
           else {
               std::cout << ERROR_AT(e) << "unknown light type " << e.attribute("v").toStdString() << std::endl;
               return false;
           }
       } else if (e.tagName() == "color") {
           if (!parseColor(e, light->color)) {
               PARSE_ERROR(e);
               return false;
           }
       } else if (e.tagName() == "function") {
           if (!parseTriple(e, light->function.x, light->function.y, light->function.z, "a", "b", "c") &&
               !parseTriple(e, light->function.x, light->function.y, light->function.z, "x", "y", "z") &&
               !parseTriple(e, light->function.x, light->function.y, light->function.z, "v1", "v2", "v3")) {
               PARSE_ERROR(e);
               return false;
           }
       } else if (e.tagName() == "position") {
           if (light->type == LightType::LIGHT_DIRECTIONAL) {
               std::cout << ERROR_AT(e) << "position is not applicable to directional lights" << std::endl;
               return false;
           }
           if (!parseTriple(e, light->pos.x, light->pos.y, light->pos.z, "x", "y", "z")) {
               PARSE_ERROR(e);
               return false;
           }
       } else if (e.tagName() == "direction") {
           if (light->type == LightType::LIGHT_POINT) {
               std::cout << ERROR_AT(e) << "direction is not applicable to point lights" << std::endl;
               return false;
           }
           if (!parseTriple(e, light->dir.x, light->dir.y, light->dir.z, "x", "y", "z")) {
               PARSE_ERROR(e);
               return false;
           }
       } else if (e.tagName() == "penumbra") {
           if (light->type != LightType::LIGHT_SPOT) {
               std::cout << ERROR_AT(e) << "penumbra is only applicable to spot lights" << std::endl;
               return false;
           }
           float penumbra = 0.f;
           if (!parseSingle(e, penumbra, "v")) {
               PARSE_ERROR(e);
               return false;
           }

           light->penumbra = penumbra * M_PI / 180.f;
       } else if (e.tagName() == "angle") {
           if (light->type != LightType::LIGHT_SPOT) {
               std::cout << ERROR_AT(e) << "angle is only applicable to spot lights" << std::endl;
               return false;
           }

           float angle = 0.f;
           if (!parseSingle(e, angle, "v")) {
               PARSE_ERROR(e);
               return false;
           }
           light->angle = angle * M_PI / 180.f;
       } else if (e.tagName() == "width") {
           if (light->type != LightType::LIGHT_AREA) {
               std::cout << ERROR_AT(e) << "width is only applicable to area lights" << std::endl;
               return false;
           }
           if (!parseSingle(e, light->width, "v")) {
               PARSE_ERROR(e);
               return false;
           }
       } else if (e.tagName() == "height") {
           if (light->type != LightType::LIGHT_AREA) {
               std::cout << ERROR_AT(e) << "height is only applicable to area lights" << std::endl;
               return false;
           }
           if (!parseSingle(e, light->height, "v")) {
               PARSE_ERROR(e);
               return false;
           }
       } else if (!e.isNull()) {
           UNSUPPORTED_ELEMENT(e);
           return false;
       }
       childNode = childNode.nextSibling();
   }

   return true;
}

/**
* Parse a <cameradata> tag and fill in m_cameraData.
*/
bool ScenefileReader::parseCameraData(const QDomElement &cameradata) {
   bool focusFound = false;
   bool lookFound = false;

   // Iterate over child elements
   QDomNode childNode = cameradata.firstChild();
   while (!childNode.isNull()) {
       QDomElement e = childNode.toElement();
       if (e.tagName() == "pos") {
           if (!parseTriple(e, m_cameraData.pos.x, m_cameraData.pos.y, m_cameraData.pos.z, "x", "y", "z")) {
               PARSE_ERROR(e);
               return false;
           }
           m_cameraData.pos.w = 1;
       } else if (e.tagName() == "look" || e.tagName() == "focus") {
           if (!parseTriple(e, m_cameraData.look.x, m_cameraData.look.y, m_cameraData.look.z, "x", "y", "z")) {
               PARSE_ERROR(e);
               return false;
           }

           if (e.tagName() == "focus") {
               // Store the focus point in the look vector (we will later subtract
               // the camera position from this to get the actual look vector)
               m_cameraData.look.w = 1;
               focusFound = true;
           } else {
               // Just store the look vector
               m_cameraData.look.w = 0;
               lookFound = true;
           }
       } else if (e.tagName() == "up") {
           if (!parseTriple(e, m_cameraData.up.x, m_cameraData.up.y, m_cameraData.up.z, "x", "y", "z")) {
               PARSE_ERROR(e);
               return false;
           }
           m_cameraData.up.w = 0;
       } else if (e.tagName() == "heightangle") {
           float heightAngle = 0.f;
           if (!parseSingle(e, heightAngle, "v")) {
               PARSE_ERROR(e);
               return false;
           }
           m_cameraData.heightAngle = heightAngle * M_PI / 180.f;
       } else if (e.tagName() == "aperture") {
           if (!parseSingle(e, m_cameraData.aperture, "v")) {
               PARSE_ERROR(e);
               return false;
           }
       } else if (e.tagName() == "focallength") {
           if (!parseSingle(e, m_cameraData.focalLength, "v")) {
               PARSE_ERROR(e);
               return false;
           }
       } else if (!e.isNull()) {
           UNSUPPORTED_ELEMENT(e);
           return false;
       }
       childNode = childNode.nextSibling();
   }

   if (focusFound && lookFound) {
       std::cout << ERROR_AT(cameradata) << "camera can not have both look and focus" << std::endl;
       return false;
   }

   if (focusFound) {
       // Convert the focus point (stored in the look vector) into a
       // look vector from the camera position to that focus point.
       m_cameraData.look -= m_cameraData.pos;
   }

   return true;
}

/**
* Parse an <object> tag and create a new CS123SceneNode in m_nodes.
*/
bool ScenefileReader::parseObjectData(const QDomElement &object) {
   if (!object.hasAttribute("name")) {
       PARSE_ERROR(object);
       return false;
   }

   if (object.attribute("type") != "tree") {
       std::cout << "top-level <object> elements must be of type tree" << std::endl;
       return false;
   }

   std::string name = object.attribute("name").toStdString();

   // Check that this object does not exist
   if (m_objects[name]) {
       std::cout << ERROR_AT(object) << "two objects with the same name: " << name << std::endl;
       return false;
   }

   // Create the object and add to the map
   SceneNode *node = new SceneNode;
   m_nodes.push_back(node);
   m_objects[name] = node;

   // Iterate over child elements
   QDomNode childNode = object.firstChild();
   while (!childNode.isNull()) {
       QDomElement e = childNode.toElement();
       if (e.tagName() == "transblock") {
           SceneNode *child = new SceneNode;
           m_nodes.push_back(child);
           if (!parseTransBlock(e, child)) {
               PARSE_ERROR(e);
               return false;
           }
           node->children.push_back(child);
       } else if (!e.isNull()) {
           UNSUPPORTED_ELEMENT(e);
           return false;
       }
       childNode = childNode.nextSibling();
   }

   return true;
}


bool ScenefileReader::parseKeyFrame(const QDomElement &keyFrame, TransformationMap& tm, std::vector<std::string>& prevOrder) {
    QDomNode childNode = keyFrame.firstChild();
    float key = keyFrame.attribute("key").toFloat();
    std::string type = keyFrame.attribute("type").toStdString();

    std::cout << "Parsing a keyframe" << std::endl;

    int frame;
    if (type == "fractional") {
        frame = (int) ceil(key * ceil(m_globalData.framerate * m_globalData.duration));
    } else {
        frame = (int) key;
    }

    std::vector<std::string> curOrder;
    while (!childNode.isNull()) {
        QDomElement e = childNode.toElement();
        if (e.tagName() == "translate") {
            SceneTransformation *t = new SceneTransformation();
            t->type = TransformationType::TRANSFORMATION_TRANSLATE;

            std::string id;
            if (!parseQuadruple(e, id, t->translate.x, t->translate.y, t->translate.z, "id", "x", "y", "z")) {
                PARSE_ERROR(e);
                delete(t);
                return false;
            }

            curOrder.push_back(id);

            if (tm.contains(id)) {
               auto& idTranslations = tm.at(id);
               idTranslations.push_back({ frame, t });
            } else {
               tm.emplace(id, std::vector<std::tuple<int, SceneTransformation*>>());
               auto& idTranslations = tm.at(id);
               if (frame != 0) {
                   SceneTransformation *defaultTranslate = new SceneTransformation();
                   defaultTranslate->type = TransformationType::TRANSFORMATION_TRANSLATE;
                   defaultTranslate->translate = glm::vec3(0.f);

                   idTranslations.push_back({ 0, defaultTranslate });
               }
               idTranslations.push_back({ frame, t });
            }

        } else if (e.tagName() == "rotate") {
            SceneTransformation *t = new SceneTransformation();
            t->type = TransformationType::TRANSFORMATION_ROTATE;

            std::string id;
            float angle;
            if (!parseQuintuple(e, id, t->rotate.x, t->rotate.y, t->rotate.z, angle, "id", "x", "y", "z", "angle")) {
                PARSE_ERROR(e);
                delete(t);
                return false;
            }
            // Convert to radians
            t->angle = angle * M_PI / 180;

            curOrder.push_back(id);

            if (tm.contains(id)) {
               auto& idRotations = tm.at(id);
               idRotations.push_back({ frame, t });
            } else {
               tm.emplace(id, std::vector<std::tuple<int, SceneTransformation*>>());
               auto& idRotations= tm.at(id);
               if (frame != 0) {
                   SceneTransformation *defaultRotate = new SceneTransformation();
                   defaultRotate->type = TransformationType::TRANSFORMATION_ROTATE;
                   defaultRotate->rotate = glm::vec3{ 0.f, 1.f, 0.f };
                   defaultRotate->angle = 0;

                   idRotations.push_back({ 0, defaultRotate });
               }
               idRotations.push_back({ frame, t });
            }
        } else if (e.tagName() == "scale") {
            SceneTransformation *t = new SceneTransformation();
            t->type = TransformationType::TRANSFORMATION_SCALE;

            std::string id;
            if (!parseQuadruple(e, id, t->scale.x, t->scale.y, t->scale.z, "id", "x", "y", "z")) {
                PARSE_ERROR(e);
                delete(t);
                return false;
            }

            curOrder.push_back(id);

            if (tm.contains(id)) {
               auto& idTranslations = tm.at(id);
               idTranslations.push_back({ frame, t });
            } else {
               tm.emplace(id, std::vector<std::tuple<int, SceneTransformation*>>());
               auto& idTranslations = tm.at(id);
               if (frame != 0) {
                   SceneTransformation *defaultScale = new SceneTransformation();
                   defaultScale->type = TransformationType::TRANSFORMATION_SCALE;
                   defaultScale->scale = glm::vec3{ 1.f, 1.f, 1.f };

                   idTranslations.push_back({ 0, defaultScale});
               }
               idTranslations.push_back({ frame, t });
            }
        } else if (e.tagName() == "matrix") {
            return false;
        }

        childNode = childNode.nextSibling();
    }

    int sizeWithDups = curOrder.size();
    curOrder.erase(unique(curOrder.begin(), curOrder.end()), curOrder.end());
    int sizeWithoutDups = curOrder.size();

    if (sizeWithDups != sizeWithoutDups) {
        std::cout << "ERROR: Duplicated id in keyframe" << std::endl;
        PARSE_ERROR(keyFrame);
        return false;
    }

    std::vector<std::string> sharedFromCurrent;
    std::vector<std::string> sharedFromPrevious;

    for (std::string &s : curOrder) {
        if (std::find(prevOrder.begin(), prevOrder.end(), s) != prevOrder.end()) {
            sharedFromCurrent.push_back(s);
        }
    }

    for (std::string &s : prevOrder) {
        if (std::find(curOrder.begin(), curOrder.end(), s) != curOrder.end()) {
            sharedFromPrevious.push_back(s);
        }
    }

    if (sharedFromCurrent != sharedFromPrevious) {
        std::cout << "ERROR: Invalid ordering of transformations" << std::endl;
        PARSE_ERROR(keyFrame);
        return false;
    }

    std::vector<std::string> newOrder;

    int prevIndex = 0;
    int curIndex = 0;

    std::vector<std::string> toInsert;

    std::cout << "Entering loop of doom" << std::endl;
    bool finished = false;
    while (!finished) {
//        std::cout << "Cur Index " << curIndex << std::endl;
//        std::cout << "Cur Order Size" << curOrder.size() << std::endl;
//        std::cout << "Prev Index " << prevIndex << std::endl;
//        std::cout << "Prev Order Size " << prevOrder.size() << std::endl;

        if (prevIndex >= prevOrder.size()) {
            for (int i = curIndex; i < curOrder.size(); i++) {
                newOrder.push_back(curOrder.at(i));
            }
            finished = true;
            continue;
        }

        if (curIndex >= curOrder.size()) {
            for (int i = prevIndex; i < prevOrder.size(); i++) {
                newOrder.push_back(prevOrder.at(i));
            }
            finished = true;
            continue;
        }

        std::string& curId = curOrder.at(curIndex);

        if (curId == prevOrder.at(prevIndex)) {
            newOrder.push_back(curId);
            prevIndex++;
            curIndex++;
            continue;
        }

        bool foundMatch = false;
        int prevOrderMatchIndex = 0;
        for (int i = prevIndex + 1; i < prevOrder.size(); i++) {
            if (curId == prevOrder.at(i)) {
                foundMatch = true;
                prevOrderMatchIndex = i;
            }
        }

        if (!foundMatch) {
            toInsert.push_back(curId);
            curIndex++;
        }

        if (foundMatch) {
            for (int i = prevIndex; i < prevOrderMatchIndex; i++) {
                newOrder.push_back(prevOrder.at(i));
            }

            for (int i = 0; i < toInsert.size(); i++) {
                newOrder.push_back(toInsert.at(i));
            }

            newOrder.push_back(prevOrder.at(prevOrderMatchIndex));

            curIndex++;
            prevIndex = prevOrderMatchIndex + 1;
        }
    }


    std::cout << "HERE" << std::endl;

    prevOrder.clear();
    for (int i = 0; i < newOrder.size(); i++) {
        prevOrder.push_back(newOrder[i]);
    }

    return true;
}

float linearInterpolate(int frame, int startingFrame, int endingFrame, float start, float end) {
    int frameDiff = endingFrame - startingFrame;
    float between = (frame - startingFrame) / (float) frameDiff;

//    std::cout << start + between * (end - start) << std::endl;

    return start + between * (end - start);
}

void ScenefileReader::interpolateTranslation(std::vector<std::tuple<int, SceneTransformation*>> &translations, SceneNode *node) {
    auto translate = [=](int f){
        for (int i = 0; i < translations.size(); i++) {
            auto [startingFrame, startingTranslation] = translations[i];
            if (f >= startingFrame) {
                if (i == translations.size() - 1) {
                    // Between startingFrame and the end of the animation - just repeat startingTranslation
                    return startingTranslation->translate;
                } else {
                    auto [endingFrame, endingTranslation] = translations[i + 1];
                    if (f < endingFrame) {
                        std::cout << endingTranslation->translate.x << std::endl;
                        // Interpolate between startingTranslation and endingTranslation
                        return glm::vec3(
                                    linearInterpolate(f, startingFrame, endingFrame, startingTranslation->translate.x, endingTranslation->translate.x),
                                    linearInterpolate(f, startingFrame, endingFrame, startingTranslation->translate.y, endingTranslation->translate.y),
                                    linearInterpolate(f, startingFrame, endingFrame, startingTranslation->translate.z, endingTranslation->translate.z));
                    }
                }
            }


        }

        std::cout << "ERROR: frame unreachable" << std::endl;
        exit(1);
    };

    InterpolatedSceneTransformation *t = new InterpolatedSceneTransformation();
    t->type = TransformationType::TRANSFORMATION_TRANSLATE;
    t->translate = translate;

    node->transformations.push_back(t);
}

void ScenefileReader::interpolateScale(std::vector<std::tuple<int, SceneTransformation*>> &scales, SceneNode *node) {
    auto scale = [=](int f){
        for (int i = 0; i < scales.size(); i++) {
            auto [startingFrame, startingScale] = scales[i];

            if (f >= startingFrame) {
                if (i == scales.size() - 1) {
                    // Between startingFrame and the end of the animation - just repeat startingScale
                    return startingScale->scale;
                } else {
                    auto [endingFrame, endingScale] = scales[i + 1];
                    if (f < endingFrame) {
                        // Interpolate between startingScale and endingScale
                        return glm::vec3(
                                    linearInterpolate(f, startingFrame, endingFrame, startingScale->scale.x, endingScale->scale.x),
                                    linearInterpolate(f, startingFrame, endingFrame, startingScale->scale.y, endingScale->scale.y),
                                    linearInterpolate(f, startingFrame, endingFrame, startingScale->scale.z, endingScale->scale.z));
                    }
                }
            }
        }

        std::cout << "ERROR: frame unreachable" << std::endl;
        exit(1);
    };

    InterpolatedSceneTransformation *t = new InterpolatedSceneTransformation();
    t->type = TransformationType::TRANSFORMATION_SCALE;
    t->scale = scale;

    node->transformations.push_back(t);
}

void ScenefileReader::interpolateRotation(std::vector<std::tuple<int, SceneTransformation*>> &rotations, SceneNode *node) {
    auto rotate = [=](int f){
        for (int i = 0; i < rotations.size(); i++) {
            auto [startingFrame, startingRotation] = rotations[i];

            if (f >= startingFrame) {
                if (i == rotations.size() - 1) {
                    // Between startingFrame and the end of the animation - just repeat startingScale
                    return startingRotation->rotate;
                } else {
                    auto [endingFrame, endingRotation] = rotations[i + 1];
                    if (f < endingFrame) {
                        // Interpolate between startingrotation and endingrotation
                        return glm::vec3(
                                    linearInterpolate(f, startingFrame, endingFrame, startingRotation->rotate.x, endingRotation->rotate.x),
                                    linearInterpolate(f, startingFrame, endingFrame, startingRotation->rotate.y, endingRotation->rotate.y),
                                    linearInterpolate(f, startingFrame, endingFrame, startingRotation->rotate.z, endingRotation->rotate.z));
                    }
                }
            }
        }

        std::cout << "ERROR: frame unreachable" << std::endl;
        exit(1);
    };

    auto angle = [=](int f){
        for (int i = 0; i < rotations.size(); i++) {
            auto [startingFrame, startingRotation] = rotations[i];

            if (f >= startingFrame) {
                if (i == rotations.size() - 1) {
                    // Between startingFrame and the end of the animation - just repeat startingScale
                    return startingRotation->angle;
                } else {
                    auto [endingFrame, endingRotation] = rotations[i + 1];
                    if (f < endingFrame) {
                        // Interpolate between startingrotation and endingrotation
                        return linearInterpolate(f, startingFrame, endingFrame, startingRotation->angle, endingRotation->angle);
                    }
                }
            }
        }

        std::cout << "ERROR: frame unreachable" << std::endl;
        exit(1);
    };

    InterpolatedSceneTransformation *t = new InterpolatedSceneTransformation();
    t->type = TransformationType::TRANSFORMATION_ROTATE;
    t->rotate = rotate;
    t->angle = angle;

    node->transformations.push_back(t);
}

/**
* Parse a <transblock> tag into node, which consists of any number of
* <translate>, <rotate>, <scale>, or <matrix> elements followed by one
* <object> element.  That <object> element is either a master reference,
* a subtree, or a primitive.  If it's a master reference, we handle it
* here, otherwise we will call other methods.  Example <transblock>:
*
* <transblock>
*   <translate x="1" y="2" z="3"/>
*   <rotate x="0" y="1" z="0" a="90"/>
*   <scale x="1" y="2" z="1"/>
*   <object type="primitive" name="sphere"/>
* </transblock>
*/
bool ScenefileReader::parseTransBlock(const QDomElement &transblock, SceneNode* node) {
   // Iterate over child elements
   QDomNode childNode = transblock.firstChild();
   std::vector<std::string> order;
   TransformationMap tm = TransformationMap();

   std::cout << "Parsing the transblock" << std::endl;

   while (!childNode.isNull()) {
       QDomElement e = childNode.toElement();
       if (e.tagName() == "keyframe") {
            if (!parseKeyFrame(e, tm, order)) {
                return false;
            }
        } else if (e.tagName() == "object") {
            if (e.attribute("type") == "master") {
               std::string masterName = e.attribute("name").toStdString();
               if (!m_objects[masterName]) {
                   std::cout << ERROR_AT(e) << "invalid master object reference: " << masterName << std::endl;
                   return false;
               }
               node->children.push_back(m_objects[masterName]);
            } else if (e.attribute("type") == "tree") {
               QDomNode subNode = e.firstChild();
               while (!subNode.isNull()) {
                   QDomElement e = subNode.toElement();
                   if (e.tagName() == "transblock") {
                       SceneNode* n = new SceneNode;
                       m_nodes.push_back(n);
                       node->children.push_back(n);
                       if (!parseTransBlock(e, n)) {
                           PARSE_ERROR(e);
                           return false;
                       }
                   } else if (!e.isNull()) {
                       UNSUPPORTED_ELEMENT(e);
                       return false;
                   }
                   subNode = subNode.nextSibling();
               }
           } else if (e.attribute("type") == "primitive") {
               if (!parsePrimitive(e, node)) {
                   PARSE_ERROR(e);
                   return false;
               }
           } else {
               std::cout << ERROR_AT(e) << "invalid object type: " << e.attribute("type").toStdString() << std::endl;
               return false;
           }
        } else if (!e.isNull()) {
           UNSUPPORTED_ELEMENT(e);
           return false;
        }
        childNode = childNode.nextSibling();
   }

   for (std::string& id : order) {
       std::vector<std::tuple<int, SceneTransformation*>> transformationsToInterpolate = tm.at(id);

       assert(transformationsToInterpolate.size() != 0);
       auto [frame, transformation] = transformationsToInterpolate[0];

       switch (transformation->type) {
       case TransformationType::TRANSFORMATION_TRANSLATE:
           interpolateTranslation(transformationsToInterpolate, node);
           break;
       case TransformationType::TRANSFORMATION_SCALE:
           interpolateScale(transformationsToInterpolate, node);
           break;
       case TransformationType::TRANSFORMATION_ROTATE:
           interpolateRotation(transformationsToInterpolate, node);
           break;
       default:
           std::cout << "Invalid transformation type" << std::endl;
           return false;
       }
   }

   return true;
}



/**
* Parse an <object type="primitive"> tag into node.
*/
bool ScenefileReader::parsePrimitive(const QDomElement &prim, SceneNode* node) {
   // Default primitive
   ScenePrimitive* primitive = new ScenePrimitive();
   SceneMaterial& mat = primitive->material;
   mat.clear();
   primitive->type = PrimitiveType::PRIMITIVE_CUBE;
   mat.textureMap.isUsed = false;
   mat.bumpMap.isUsed = false;
   mat.cDiffuse.r = mat.cDiffuse.g = mat.cDiffuse.b = 1;
   node->primitives.push_back(primitive);

   std::filesystem::path basepath = std::filesystem::path(file_name).parent_path().parent_path();

   // Parse primitive type
   std::string primType = prim.attribute("name").toStdString();
   if (primType == "sphere") primitive->type = PrimitiveType::PRIMITIVE_SPHERE;
   else if (primType == "cube") primitive->type = PrimitiveType::PRIMITIVE_CUBE;
   else if (primType == "cylinder") primitive->type = PrimitiveType::PRIMITIVE_CYLINDER;
   else if (primType == "cone") primitive->type = PrimitiveType::PRIMITIVE_CONE;
   else if (primType == "torus") primitive->type = PrimitiveType::PRIMITIVE_TORUS;
   else if (primType == "mesh") {
       primitive->type = PrimitiveType::PRIMITIVE_MESH;
       if (prim.hasAttribute("meshfile")) {
           std::filesystem::path relativePath(prim.attribute("meshfile").toStdString());
           primitive->meshfile = (basepath / relativePath).string();
       } else if (prim.hasAttribute("filename")) {
           std::filesystem::path relativePath(prim.attribute("filename").toStdString());
           primitive->meshfile = (basepath / relativePath).string();
       } else {
           std::cout << "mesh object must specify filename" << std::endl;
           return false;
       }
   }

   // Iterate over child elements
   QDomNode childNode = prim.firstChild();
   while (!childNode.isNull()) {
       QDomElement e = childNode.toElement();
       if (e.tagName() == "diffuse") {
           if (!parseColor(e, mat.cDiffuse)) {
               PARSE_ERROR(e);
               return false;
           }
       } else if (e.tagName() == "ambient") {
           if (!parseColor(e, mat.cAmbient)) {
               PARSE_ERROR(e);
               return false;
           }
       } else if (e.tagName() == "reflective") {
           if (!parseColor(e, mat.cReflective)) {
               PARSE_ERROR(e);
               return false;
           }
       } else if (e.tagName() == "specular") {
           if (!parseColor(e, mat.cSpecular)) {
               PARSE_ERROR(e);
               return false;
           }
       } else if (e.tagName() == "emissive") {
           if (!parseColor(e, mat.cEmissive)) {
               PARSE_ERROR(e);
               return false;
           }
       } else if (e.tagName() == "transparent") {
           if (!parseColor(e, mat.cTransparent)) {
               PARSE_ERROR(e);
               return false;
           }
       } else if (e.tagName() == "shininess") {
           if (!parseSingle(e, mat.shininess, "v")) {
               PARSE_ERROR(e);
               return false;
           }
       } else if (e.tagName() == "ior") {
           if (!parseSingle(e, mat.ior, "v")) {
               PARSE_ERROR(e);
               return false;
           }
       } else if (e.tagName() == "texture") {
           if (!parseMap(e, mat.textureMap, basepath)) {
               PARSE_ERROR(e);
               return false;
           }
       } else if (e.tagName() == "bumpmap") {
           if (!parseMap(e, mat.bumpMap, basepath)) {
               PARSE_ERROR(e);
               return false;
           }
       } else if (e.tagName() == "blend") {
           if (!parseSingle(e, mat.blend, "v")) {
               PARSE_ERROR(e);
               return false;
           }
       } else {
           UNSUPPORTED_ELEMENT(e);
           return false;
       }
       childNode = childNode.nextSibling();
   }

   return true;
}
