//
// Created by Ext Culith on 2025/7/26.
//

#ifndef CYAN_POLYPARTITION_H
#define CYAN_POLYPARTITION_H

#include "../Simplification/Simplification.h"

namespace cyanvne::platform::algorithm::partitioning
{
    using simplification::Contour;
    using simplification::ContourList;

    using ConvexPolygon = std::vector<glm::vec2>;
    using PartitionedShape = std::vector<ConvexPolygon>;

    enum class PartitionAlgorithm
    {
        // Hertel-Mehlhorn: Fast, good quality results. Ideal for physics.
        HM,
        // Optimal Convex Partition: Guarantees minimum number of convex parts, but is very slow (O(n^3)).
        OPT,
        // Ear Clipping Triangulation: Standard, robust triangulation method.
        Triangulate_EC,
        // Monotone Triangulation: Faster triangulation, but can produce lower quality (thin) triangles.
        Triangulate_MONO
    };

    /**
     * @brief Partitions a list of contours (potentially with holes) into convex polygons or triangles.
     * This is the main entry point for the partitioning module.
     * @param source_contours A list of contours, typically from Suzuki's algorithm, which preserves hierarchy.
     * @param algorithm The partitioning or triangulation algorithm to use.
     * @return PartitionedShape A list containing all the final convex polygons or triangles.
     */
    PartitionedShape partition_contours(
        const ContourList& source_contours,
        PartitionAlgorithm algorithm = PartitionAlgorithm::HM);

} // namespace cyanvne::platform::algorithm::partitioning

#endif
