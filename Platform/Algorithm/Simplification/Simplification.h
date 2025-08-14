//
// Created by Ext Culith on 2025/7/26.
//

#ifndef SIMPLIFICATION_H
#define SIMPLIFICATION_H

#include "../ContourTracing/ContourTracing.h"

namespace cyanvne::platform::algorithm::simplification
{
    using contourtracing::Contour;
    using contourtracing::ContourList;

    enum class SimplificationAlgorithm
    {
        RDP, // Ramer-Douglas-Peucker
        VisvalingamWhyatt // Visvalingam-Whyatt
    };

    /**
     * Simplifies a contour using the Ramer-Douglas-Peucker algorithm.
     * @param source_contour The contour to simplify.
     * @param epsilon The distance threshold for simplification.
     * @return A simplified contour.
     */
    Contour simplify_contour_rdp(const Contour& source_contour, float epsilon);

    /**
     * Simplifies a contour using the Visvalingam-Whyatt algorithm.
     * @param source_contour The contour to simplify.
     * @param epsilon The area threshold for simplification.
     * @return A simplified contour.
     */
    Contour simplify_contour_vw(const Contour& source_contour, float epsilon);

    /**
     * Simplifies a contour using the specified algorithm.
     * @param source_contour The contour to simplify.
     * @param algorithm The simplification algorithm to use.
     * @param epsilon The threshold for simplification.
     * @return A simplified contour.
     */
    Contour simplify_contour(
        const Contour& source_contour,
        SimplificationAlgorithm algorithm,
        float epsilon);

    /**
     * Simplifies a list of contours using the specified algorithm.
     * @param source_contours The list of contours to simplify.
     * @param algorithm The simplification algorithm to use.
     * @param epsilon The threshold for simplification.
     * @return A list of simplified contours.
     */
    ContourList simplify_contours(
        const ContourList& source_contours,
        SimplificationAlgorithm algorithm,
        float epsilon);

} // namespace cyanvne::platform::algorithm::simplification

#endif //SIMPLIFICATION_H
