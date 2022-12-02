#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <map>

#include "utils/scenedata.h"
#include "primitives/worldprimitive.h"
#include "lights.h"
#include "texture/texture.h"

// You are NOT supposed to modify this file.
glm::vec4 phong(
           const glm::vec3& position,
           glm::vec3 normal,
           glm::vec3 directionToCamera,
           const SceneMaterial  &material,
           const std::tuple<float, float>& uv,
           const std::map<std::string, Texture::Texture>& textures,
           const std::vector<Lights::Proxy>& lights,
           const SceneGlobalData& globals,
           const std::vector<WorldPrimitive::Proxy>& prims,
           const bool enableShadow,
           const bool enableTexture);
