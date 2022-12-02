#include "colorutils.h"

#include <cstdint>
#include <cmath>
#include "rgba.h"

namespace ColorUtils {
    /**
     * @brief intToIntensity - converts a 0-255 int value to a 0-1 float
     * @param i - an 8-bit unsigned integer
     * @return that integer represented as a float on the scale 0-1
     */
    float intToIntensity(std::uint8_t i) {
        return i / 255.f;
    }

    /**
     * @brief intensityToInt - converts a 0-1 float to a 0-255 flaot
     * @param f - some float on the range 0-1
     * @return the float converted from 0-1 to an unsigend 8-bit int 0-255
     */
    std::uint8_t intensityToInt(float f) {
        return round(f * 255);
    }

    /**
     * @brief rgbaToGray - converts an rgba pixel to a single 0-255 float value using the luma method
     * @param pixel - a reference to a RGBA pixer
     * @return a single grey intensity of that pixel
     */
    float rgbaToGray(const RGBA &pixel) {
        // using the luma method
        return ((0.299 * pixel.r) + (0.587 * pixel.g) + (0.114 * pixel.b));
    }

    /**
     * @brief Helper function to clamp a float intensity to a 0-255 value
     * 
     * @param intensity - some float representing the intensity of a color
     * @return - the same color but on the range 0-255
     */
    std::uint8_t clampIntensity(float intensity) {
        return 255 * fmin(fmax(0, intensity), 1);
    }

    /**
     * @brief Convert a vector representation of a color into a RGBA struct
     * 
     * @param illumination - a 4d vector representing an RGBA color
     * @return RGBA 
     */
    RGBA toRGBA(const glm::vec4 &illumination) {
        std::uint8_t rVal = clampIntensity(illumination.r);
        std::uint8_t gVal = clampIntensity(illumination.g);
        std::uint8_t bVal = clampIntensity(illumination.b);

        return RGBA{ rVal, gVal, bVal };
    }

    glm::vec4 toIntensity(const RGBA& color) {
        return { intToIntensity(color.r), intToIntensity(color.g), intToIntensity(color.b), intToIntensity(color.a) };
    }
}
