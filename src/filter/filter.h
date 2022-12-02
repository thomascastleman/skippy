#ifndef FILTER_H
#define FILTER_H

#include <vector>
#include "utils/rgba.h"

class Filter {
public:
    Filter();

    static void applyBlur(RGBA *image, int width, int height, int radius);
private:
    template <typename T>
    static void convolveHorizontal(const T *data, int width, int height, T *result, const std::vector<float> &horizontalKernel);

    template <typename T>
    static void convolveVertical(const T* data, int width, int height, T *result, const std::vector<float> &verticalKernel);
};

#endif // FILTER_H
