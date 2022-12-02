#include "filter.h"

#include "utils/colorutils.h"
#include <cmath>

Filter::Filter() { }

/**
 * @brief createAccs - A templated function that creates the correct number of accumulators for a given convolution
 * @param accs - A vector of floats to be used as accumulators
 */
template <typename T>
void createAccs(std::vector<float> &accs);

/**
 * @brief createAccs - Creates 3 accumulators to keep track of R, G, and B values.
 * @param accs - A vector of floats
 */
template <>
void createAccs<RGBA>(std::vector<float> &accs) {
    accs.clear();
    accs.reserve(3);

    accs.push_back(0.f);
    accs.push_back(0.f);
    accs.push_back(0.f);
}

/**
 * @brief createAccs - Creates one accumulator to keep track of intensity values
 * @param accs - A vector of floats.
 */
template <>
void createAccs<float>(std::vector<float> &accs) {
    accs.clear();
    accs.reserve(1);

    accs.push_back(0.f);
}


/**
 * @brief incrementAccs - A templated function that properly increments some accumulators based on the pixel type and kernel weight.
 * @param accs - A vector of floats representing the accumulator values
 * @param addition - The value you want to add to the accumulators.
 * @param coefficient - The coeefficent you want to multiply the added values by when updated the accumulators.
 */
template <typename T>
void incrementAccs(std::vector<float> &accs, const T &addition, float coefficient);

/**
 * @brief incrementAccs - Updates the three accumulators based on the RGB values of a pixel and some coefficient
 * @param accs - A vector of three floats representing the R, G, and B accumulated values
 * @param addition - Some RGBA pixel
 * @param coefficient - The weight of the kernel
 */
template <>
void incrementAccs<RGBA>(std::vector<float> &accs, const RGBA &addition, float coefficient) {
    accs[0] += coefficient * addition.r;
    accs[1] += coefficient * addition.g;
    accs[2] += coefficient * addition.b;
}

/**
 * @brief incrementAccs - Updates one accumulator based on the intensity value of a pixel and some coefficient
 * @param accs - A vector of a single float representing the accumulation of intensity values
 * @param addition - Some pixel intensity
 * @param coefficient - The weight of the kernel
 */
template <>
void incrementAccs<float>(std::vector<float> &accs, const float &addition, float coefficient) {
    accs[0] += coefficient * addition;
}

/**
 * @brief createPixel - Templated function taht, given some accumulators, this returns the proper type of pixel created from those accumulators
 * @param accs - The accumulators from which to create the pixel
 * @return Some type of pixel based ont eh accumulators
 */
template <typename T>
T createPixel(const std::vector<float> &accs);

/**
 * @brief createPixel - Creates an RGBA pixel based on 3 accumulators
 * @param accs - A vector of floats representing the desired R, G, and B values
 * @return An RGBA value based on the accumulators.
 */
template <>
RGBA createPixel(const std::vector<float> &accs) {
    std::uint8_t redVal = accs[0];
    std::uint8_t greenVal = accs[1];
    std::uint8_t blueVal = accs[2];

    return { redVal, greenVal, blueVal, 255 };
}

/**
 * @brief createPixel - Creates an intensity pixel based on a single accumulator
 * @param accs - A vector of floats containing a single float representing the intensity of the pixel
 * @return A float value representing the intensity of a pixel
 */
template <>
float createPixel(const std::vector<float> &accs) {
    return accs[0];
}

/**
 * @brief Filter::convolveHorizontal - A templated function taking in either an image of RGBA data or an image of float intensity data, convolves the original image by the
 * given kernel and saves the resul in the result vector. Only convolves the rows.
 * @param data - The data of the original image either vector or RGBA or vector of float values.
 * @param width - The width of the image
 * @param height - The height of the image
 * @param result - The vector in which to store the convolved image.
 * @param horizontalKernel - Some vector by which to horizontally convolve the image.
 */
template <typename T>
void Filter::convolveHorizontal(const T *data, int width, int height, T *result, const std::vector<float> &horizontalKernel) {
    int lastKernelIndex = horizontalKernel.size() - 1;
    int kernelRadius = horizontalKernel.size() / 2;

    // create the proper number of accumulators based on the type
    std::vector<float> accs;
    createAccs<T>(accs);

    int index = 0;
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            // zero out the accumulators
            for (float& acc : accs) {
                acc = 0.f;
            }

            // index the kernel from back to front to similate rotating 180 degrees.
            int kernelIndex = lastKernelIndex;

             // move left and right around the center pixel
            for (int colOffset = -kernelRadius; colOffset <= kernelRadius; ++colOffset) {
                float curKernelWeight = horizontalKernel[kernelIndex];
                int relCol = col + colOffset;

                // get the color (or intensity) data at this pixel, reflecting if out of bounds
                const T &curColor = ColorUtils::getReflected<T>(data, row, relCol, width, height);

                // update the accumulators with this new color data
                incrementAccs<T>(accs, curColor, curKernelWeight);

                --kernelIndex;
            }

            // add the new pixel defined by the accumulators to the result image.
            T newPixel = createPixel<T>(accs);
            result[index++] = newPixel;
        }
    }
}

/**
 * @brief Filter::convolveVector - A templated function taking in either an image of RGBA data or an image of float intensity data, convolves the original image by the
 * given kernel and saves the resul in the result vector. Only convolves the columns.
 * @param data - The data of the original image either vector or RGBA or vector of float values.
 * @param width - The width of the image
 * @param height - The height of the image
 * @param result - The vector in which to store the convolved image.
 * @param verticalKernel - Some vector by which to vertically convolve the image.
 */
template <typename T>
void Filter::convolveVertical(const T* data, int width, int height, T *result, const std::vector<float> &verticalKernel) {
    int lastKernelIndex = verticalKernel.size() - 1;
    int kernelRadius = verticalKernel.size() / 2;

    // create the proper number of accumulators based on the type
    std::vector<float> accs;
    createAccs<T>(accs);

    int index = 0;
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            // zero out the accumulators
            for (float& acc : accs) {
                acc = 0.f;
            }

            // index the kernel from back to front to similate rotating 180 degrees.
            int kernelIndex = lastKernelIndex;

            // move up and down around the center pixel
            for (int rowOffset = -kernelRadius; rowOffset <= kernelRadius; ++rowOffset) {
                float curKernelWeight = verticalKernel[kernelIndex];
                int relRow = row + rowOffset;

                // get the color (or intensity) data at this pixel, reflecting if out of bounds
                const T &curColor = ColorUtils::getReflected<T>(data, relRow, col, width, height);
                incrementAccs<T>(accs, curColor, curKernelWeight);

                --kernelIndex;
            }

            T newPixel = createPixel<T>(accs);
            result[index++] = newPixel;
        }
    }
}

/**
 * @brief triangleValueAt - given some radius and some x, this computes the proper result of the triangle function at x
 * @param x - the position in the triangle function
 * @param radius - the radius of the triangle
 * @return the float value in the triangle at the x position
 */
float triangleValueAt(float x, float radius) {
  return 1 - abs((1.f/radius) * x);
}

/**
 * @brief fillTriangleKernel - given the current blur radius, fills out the 1D triangle kernel
 * @param kernel - the kernel to populate with the triangle data
 */
void fillTriangleKernel(std::vector<float> &kernel, int radius) {
    kernel.clear();
    kernel.reserve(2*radius + 1);
    float kernelSum = 0;

    for (int x = -radius; x <= radius; ++x) {
        float curKernelVal = triangleValueAt(x, radius);
        kernelSum += curKernelVal;
        kernel.push_back(curKernelVal);
    }

    // normalize the triangle kernel
    float normalizingConstant = 1 / kernelSum;
    for (int i = 0; i < kernel.size(); i++) {
        kernel[i] = normalizingConstant * kernel[i];
    }

}

/**
 * @brief Filter::applyBlur - apply the blur filter with the current blur radius in settings to some input image
 * @param image - the input image to blue
 * @param width - the width of the image
 * @param height - the height of the image
 */
void Filter::applyBlur(RGBA *image, int width, int height, int radius) {
    std::vector<float> triangleKernel;
    fillTriangleKernel(triangleKernel, radius);

    // first horizontally convolve the original image
    RGBA horizontalBlur[height][width];
    convolveHorizontal<RGBA>(image, width, height, &horizontalBlur[0][0], triangleKernel);

    // then vertically blur the result of the first convolution and save it in the original image
    convolveVertical<RGBA>(&horizontalBlur[0][0], width, height, image, triangleKernel);
}
