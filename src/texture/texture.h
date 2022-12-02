#pragma once

#include "utils/rgba.h"
#include "utils/scenedata.h"

#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <glm/glm.hpp>

namespace Texture {
    struct Texture {
        int width;
        int height;
        std::vector<RGBA> img;
    };

    void load(const std::string filename, std::map<std::string, Texture>& textures);
    glm::vec4 getPixel(const std::tuple<float, float>& uv, const Texture& texture, const SceneMaterial& material);
}
