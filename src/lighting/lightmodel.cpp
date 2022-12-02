#include "lightmodel.h"

#include <cmath>
#include <iostream>

glm::vec3 reflectAround(glm::vec3 &inDir, glm::vec3 &reflectionAxis) {
    return glm::normalize(inDir - 2 * glm::dot(inDir, reflectionAxis) * reflectionAxis);
}

// Calculates the RGBA of a pixel from intersection infomation and globally-defined coefficients
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
        const bool enableTexture) {

    // Normalizing directions
    normal            = glm::normalize(normal);
    directionToCamera = glm::normalize(directionToCamera);

    // Output illumination (we can ignore opacity)
    glm::vec4 illumination(0, 0, 0, 1);

    glm::vec4 ambient = globals.ka * material.cAmbient;
    illumination += ambient;

    for (auto& light : lights) {
        auto [ lightToIntersect, lightColor, visible ] = light(position, prims, enableShadow);

        if (!visible) {
            continue;
        }

        // diffuse
        glm::vec3 intersectToLight = -lightToIntersect;
        float diffuseAngle = glm::dot(normal, intersectToLight);
        if (diffuseAngle < 0) {
            diffuseAngle = 0;
        }

        glm::vec4 diffuse = diffuseAngle * glm::vec4{ 1.f, 1.f, 1.f, 1.f };

        if (enableTexture && material.textureMap.isUsed) {
            glm::vec4 textureColor = Texture::getPixel(uv, textures.at(material.textureMap.filename), material);

            diffuse *= ((1 - material.blend) * globals.kd * material.cDiffuse) + (material.blend * textureColor);
        } else {
            diffuse *= globals.kd * material.cDiffuse;
        }

        // specular
        glm::vec3 mirrorDir = reflectAround(lightToIntersect, normal);
        float specularAngle = glm::dot(mirrorDir, directionToCamera);

        if (specularAngle < 0) {
            specularAngle = 0;
        } else {
            specularAngle = pow(specularAngle, material.shininess);
        }

        glm::vec4 specular = globals.ks * material.cSpecular * specularAngle;

        illumination += lightColor * (diffuse + specular);
    }

    return illumination;
}
