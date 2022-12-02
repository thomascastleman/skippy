#include "intersection.h"

#include <algorithm>
#include <iostream>
#include <optional>

namespace Intersection {
    /**
     * @brief returns a boolean denoting whether i1 has a smaller t value than i2
     * 
     * @param i1 A reference to an intersection tuple
     * @param i2 A referenct to an intersection tuple
     * @return true - if i1's t value is less than i2's
     * @return false - otherwise
     */
    bool closer(const Intersection& i1, const Intersection& i2) {
        auto& [ t1, norm1, uv1 ] = i1;
        auto& [ t2, norm2, uv2 ] = i2;

        return t1 < t2;
    }
    
    /**
     * @brief same as "closer" but runs on MaterialIntersection tuples instread
     */
    bool mCloser(const MaterialIntersection& mi1, const MaterialIntersection& mi2) {
        auto& [ i1, m1 ] = mi1;
        auto& [ i2, m3 ] = mi2;

        return closer(i1, i2);
    }

    /**
     * @brief Get the closest intersection to camera from a vector of intersections
     * 
     * @param intersections reference to vector of intersections
     * @return Intersection& - reference to closester intersection to camera
     */
    Intersection& getFirst(std::vector<Intersection>& intersections) {
        return *std::min_element(intersections.begin(), intersections.end(), closer);
     }

    /**
     * @brief Get the closest material intersection to camera from a vector of material intersections
     *
     * @param intersections reference to vector of material intersections
     * @return MaterialIntersection& - reference to closester intersection to camera
     */
    MaterialIntersection& getFirst(std::vector<MaterialIntersection>& intersections) {
        return *std::min_element(intersections.begin(), intersections.end(), mCloser);
    }
}
