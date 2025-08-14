//
// Created by Ext Culith on 2025/7/26.
//

#ifndef CONTOURTRACING_H
#define CONTOURTRACING_H

#include <vector>
#include <glm/glm.hpp>
#include "../Binarization/Binarization.h"

namespace cyanvne::platform::algorithm::contourtracing
{
    struct Contour
    {
        std::vector<glm::ivec2> points;
        // The index of the parent contour in the list, -1 if it has no parent
        int parent_id = -1;
        // Symbolic hole flag, true if this contour is a hole
        bool is_hole = false;
    };

    using ContourList = std::vector<Contour>;

    enum class ContourTracingAlgorithm
    {
        Square,   // Square contour tracing algorithm
        Moore,    // Moore neighborhood contour tracing algorithm
        Pavlidis, // Pavlidis contour tracing algorithm
        Suzuki    // Suzuki contour tracing algorithm
    };

    namespace internal
    {
        const glm::ivec2 MOORE_NEIGHBORS[8] = {
            {1, 0}, {1, -1}, {0, -1}, {-1, -1},
            {-1, 0}, {-1, 1}, {0, 1}, {1, 1}
        };
        const int MOORE_START_INDEX[8] = {6, 6, 0, 0, 2, 2, 4, 4};

        const glm::ivec2 SQUARE_NEIGHBORS[4] = {
            {0, -1}, {1, 0}, {0, 1}, {-1, 0}
        };
    } // namespace internal

    /**
     * may DEPRECATED
     * Traces the contours of a binarized grid using the square contour tracing algorithm.
     * @param binarized_grid The BinarizationResult containing the grid to trace.
     * @return A ContourList containing the traced contours.
     */
    ContourList trace_contour_square(const binarization::BinarizationResult& binarized_grid);

    /**
     * Traces the contours of a binarized grid using Moore's neighborhood tracing algorithm.
     * @param binarized_grid The BinarizationResult containing the grid to trace.
     * @return A ContourList containing the traced contours.
     */
    ContourList trace_contour_moore(const binarization::BinarizationResult& binarized_grid);

    /**
     * Traces the contours of a binarized grid using Pavlidis' contour tracing algorithm.
     * @param binarized_grid The BinarizationResult containing the grid to trace.
     * @return A ContourList containing the traced contours.
     */
    ContourList trace_contour_pavlidis(const binarization::BinarizationResult& binarized_grid);

    /**
     * Traces the contours of a binarized grid using Suzuki's contour tracing algorithm.
     * @param binarized_grid The BinarizationResult containing the grid to trace.
     * @return A ContourList containing the traced contours.
     */
    ContourList trace_contours_suzuki(const binarization::BinarizationResult& binarized_grid);

    /**
     * Traces the contours of a binarized grid using the specified contour tracing algorithm.
     * @param binarized_grid The BinarizationResult containing the grid to trace.
     * @param algorithm The contour tracing algorithm to use (default is Square).
     * @return A ContourList containing the traced contours.
     */
    ContourList trace_contours(
        const binarization::BinarizationResult& binarized_grid,
        ContourTracingAlgorithm algorithm = ContourTracingAlgorithm::Square);
}


#endif //CONTOURTRACING_H
