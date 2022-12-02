#ifndef COLORUTILS_H
#define COLORUTILS_H

#include <cstdint>
#include <vector>
#include "rgba.h"

#include <glm/glm.hpp>
#include <iostream>

namespace ColorUtils {
    float intToIntensity(std::uint8_t i);
    std::uint8_t intensityToInt(float f);
    float rgbaToGray(const RGBA &pixel);
    RGBA toRGBA(const glm::vec4 &illumination);
    glm::vec4 toIntensity(const RGBA& color);

    /**
     * @brief getReflected - according to StackOverflow, this needs to be in the .h filed because of weird templating factors
     * this function get the current pixel in an image and reflects over the edge if the pixel is out of bounds.
     * @param data - the image to get data from (float or RGBA)
     * @param row - the row of the pixel
     * @param col - the column of the pixel
     * @param width - the width of the image
     * @param height - the height of the image
     * @return the current pixel reflectd over the edge if out of bounds (float or RGBA)
     */
    template <typename T>
    const T &getReflected(const T* data, int row, int col, int width, int height) {
        int newCol;
        if (col < 0) {
            newCol = -col;
        } else if (col >= width) {
            newCol = (width - 1) - (col - width);
        } else {
            newCol = col;
        }

        int newRow;
        if (row < 0) {
            newRow = -row;
        } else if (row >= height) {
            newRow = (height - 1) - (row - height);
        } else {
            newRow = row;
        }

        return data[width * newRow + newCol];
    }
}

#endif // COLORUTILS_H
