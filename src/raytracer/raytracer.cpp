#include "raytracer.h"
#include "lighting/lightmodel.h"
#include "primitives/worldprimitive.h"
#include "filter/filter.h"
#include "utils/colorutils.h"
#include "raytracerhelper.h"

#include <QtConcurrent>

RayTracer::RayTracer(Config config) :
    m_config(config)
{}


/**
 * @brief Given a ray and a scene, find the color vector that should be rendered
 * 
 * @param ray - Some ray in world space
 * @param scene - Data about the render scene
 * @return glm::vec4 - A 4d vector representing the RGBA of the ray's intersection color
 */
glm::vec4 RayTracer::traceRay(const Ray &ray, const RayTraceScene &scene, const int depth) {
    const SceneGlobalData &globalData = scene.getGlobalData();
    std::vector<Intersection::MaterialIntersection> intersections = RayTracerHelper::getIntersections(ray, scene.getPrims());

    if (!intersections.empty()) {
        // find the closest intersection of all the primitives
        auto& [ intersection, material ] = Intersection::getFirst(intersections);
        auto& [ t, normal, uv ] = intersection;

        const glm::vec3 pt = ray.getPoint(t);

        const glm::vec4 phongLighting = phong(
                    pt,
                    normal,
                    -ray.getDir(),
                    material,
                    uv,
                    scene.getTextures(),
                    scene.getLights(),
                    scene.getGlobalData(),
                    scene.getPrims(),
                    m_config.enableShadow,
                    m_config.enableTextureMap);

        if (!m_config.enableReflection || glm::all(glm::equal(material.cReflective, glm::vec4(0.f))) || depth == 4) {
            return phongLighting;
        }

        // trace a recursive reflective ray
        glm::vec3 reflectedDir = glm::normalize(glm::reflect(ray.getDir(), normal));
        Ray recursiveRay = Ray(pt + (0.001f * reflectedDir), reflectedDir);
        glm::vec4 reflectedLight = scene.getGlobalData().ks * material.cReflective * traceRay(recursiveRay, scene, depth + 1);

        // the total light is the light at this point + the light of the reflected ray.
        return phongLighting + reflectedLight;
     } else {
        return { 0, 0, 0, 0 };
    }
}

/**
 * @brief Given a pointer to an image and a scene, it renders the scene into the image
 * 
 * @param imageData - pointer to some RGBA image
 * @param scene - A reference to a RayTraceScene
 */
void RayTracer::render(RGBA *imageData, const RayTraceScene &scene) {
    int sceneWidth = scene.width();
    int sceneHeight = scene.height();

    const Camera &camera = scene.getCamera();
    float V = 2 * tan(camera.getHeightAngle() / 2.0); // depth = 1
    float U = V * camera.getAspectRatio();

    // take 1 sample if not super-sampling, otherwise take the specified number of samples
    int numSamples = m_config.enableSuperSample ? m_config.numSamples: 1;

    auto fillIndex = [&](int index) {
        int row = index / sceneWidth;
        int col = index % sceneWidth;

        glm::vec4 accumulator = glm::vec4{ 0.f, 0.f, 0.f, 0.f };
        for (int sampleNum = 0; sampleNum < numSamples; sampleNum++) {
            // generate a pixel offset (0 - 1) for stochastic super-sampling
            // ensure that 1 sample goes directly through the center of the pixel
            float randXOffset = (sampleNum == numSamples - 1) ? 0.5f : ((float) (std::rand()) / (float) RAND_MAX);
            float randYOffset = (sampleNum == numSamples - 1) ? 0.5f : ((float) (std::rand()) / (float) RAND_MAX);

            float y = (((float) (sceneHeight - 1 - row + randYOffset)) / sceneHeight) - 0.5;
            float x = (((float) col + randXOffset) / sceneWidth) - 0.5;


            glm::vec3 eye = glm::vec3(0, 0, 0);
            glm::vec3 d = glm::normalize(glm::vec3(U*x, V*y, -1));

            // create the ray and change it from camera to world space
            Ray ray = Ray(eye, d);
            ray.transform(camera.getInverseViewMatrix());

            accumulator += traceRay(ray, scene);
        }

        // take the average of all samples
        accumulator /= numSamples;

        // finally convert it to a color and add it to the image.
        imageData[index] = ColorUtils::toRGBA(accumulator);
    };


    // use QtConcurrent to run ray-tracing concurrently;
    if (m_config.enableParallelism) {
        QVector<int> indecies;
        for (int i = 0; i < sceneWidth * sceneHeight; i++) {
            indecies.append(i);
        }

        QtConcurrent::blockingMap(indecies, fillIndex);
    } else {
        // otherwise just use on single thread.
        for (int i = 0; i < sceneWidth * sceneHeight; i++) {
            fillIndex(i);
        }
    }

    // apply a little blur at the end if post-processing is enabled to remove some noise
    if (m_config.enablePostProcess) {
        Filter::applyBlur(imageData, sceneWidth, sceneHeight, 1);
    }
}
