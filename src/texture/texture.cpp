#include "texture.h"

#include <QImage>
#include <iostream>

#include "utils/colorutils.h"

/**
 * @brief Texture::load - Given a filenam and a map of filenames to Textures, load the texture image into the map
 * @param filename - the filename pointing to the texture image
 * @param textures - a map of file strings to textures
 */
void Texture::load(const std::string filename, std::map<std::string, Texture>& textures) {
    const QString file = QString(filename.c_str());
    QImage textureImg;

    if (!textureImg.load(file)) {
        std::cout << "Critical ERROR: Failed to load texture." << std::endl;
        exit(1);
    }

    textureImg = textureImg.convertToFormat(QImage::Format_RGBX8888);
    QByteArray textureBytes = QByteArray::fromRawData((const char*) textureImg.bits(), textureImg.sizeInBytes());

    const int width = textureImg.width();
    const int height = textureImg.height();

    textures[filename] = Texture{ width, height, std::vector<RGBA>() };
    Texture& texture = textures[filename];

    texture.img.reserve(textureImg.width() * textureImg.height());

    for (int i = 0; i < textureBytes.size() / 4.f; i++){
       texture.img.push_back(RGBA{(std::uint8_t) textureBytes[4*i], (std::uint8_t) textureBytes[4*i+1], (std::uint8_t) textureBytes[4*i+2], (std::uint8_t) textureBytes[4*i+3]});
    }

    return;
}

/**
 * @brief Texture::getPixel - get the pixel color in intensity form given a uv value, a texture, and a material
 * @param uv - The uv location of the texture map
 * @param texture - the texture to draw the colors from (includes width, height, and img)
 * @param material - the material to find how many times to repeat the texture
 * @return The color of the texture at that uv
 */
glm::vec4 Texture::getPixel(const std::tuple<float, float>& uv, const Texture& texture, const SceneMaterial& material) {
    auto& [u, v] = uv;

    int col = (int) floor(u * texture.width * material.textureMap.repeatU) % texture.width;
    int row = (int) floor((1 - v) * texture.height * material.textureMap.repeatV) % texture.height;

    return ColorUtils::toIntensity(texture.img.at(row * texture.width + col));
}
