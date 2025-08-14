//
// Created by Ext Culith on 2025/7/26.
//


#include "Polypartition.h"
#include "src/Polypartition.h"

#include "Core/Logger/Logger.h"
#include "Platform/Algorithm/ContourTracing/ContourTracing.h"

namespace cyanvne::platform::algorithm::partitioning
{
    namespace internal
    {
        /**
         * @brief Converts our Contour structure to PolyPartition's TPPLPoly structure.
         * It also uses the library's own functions to set the correct vertex orientation.
         * @param contour The source contour.
         * @return A TPPLPoly object ready for processing.
         */
        TPPLPoly to_tppl_poly(const contourtracing::Contour& contour)
        {
            TPPLPoly poly;
            if (contour.points.empty())
                return poly;

            poly.Init(contour.points.size());
            for (size_t i = 0; i < contour.points.size(); ++i)
            {
                // TPPLPoint uses tppl_float, which defaults to double.
                poly[i].x = static_cast<tppl_float>(contour.points[i].x);
                poly[i].y = static_cast<tppl_float>(contour.points[i].y);
            }

            poly.SetHole(contour.is_hole);

            // Non-holes must be CCW, holes must be CW.
            if (contour.is_hole)
            {
                poly.SetOrientation(TPPL_ORIENTATION_CW);
            }
            else
            {
                poly.SetOrientation(TPPL_ORIENTATION_CCW);
            }

            return poly;
        }

        /**
         * @brief Converts a TPPLPoly object back to our ConvexPolygon (vector of glm::vec2).
         * @param poly The source TPPLPoly object.
         * @return A ConvexPolygon.
         */
        ConvexPolygon from_tppl_poly(const TPPLPoly& poly)
        {
            ConvexPolygon convex_poly;
            convex_poly.reserve(poly.GetNumPoints());
            for (long i = 0; i < poly.GetNumPoints(); ++i)
            {
                convex_poly.emplace_back(
                    static_cast<float>(poly[i].x),
                    static_cast<float>(poly[i].y)
                );
            }
            return convex_poly;
        }

    } // namespace internal

    PartitionedShape partition_contours(
        const ContourList& source_contours,
        PartitionAlgorithm algorithm)
    {
        if (source_contours.empty())
            return {};

        PartitionedShape final_polygons;
        TPPLPartition pp;

        for (size_t i = 0; i < source_contours.size(); ++i)
        {
            if (source_contours[i].parent_id != -1)
            {
                continue; // This is a hole, it will be processed with its parent.
            }


            TPPLPolyList input_poly_list;

            // Add the outer contour
            Contour outer_contour = source_contours[i];
            input_poly_list.push_back(internal::to_tppl_poly(outer_contour));

            // Find and add all holes belonging to this outer contour
            for (size_t j = 0; j < source_contours.size(); ++j)
            {
                if (source_contours[j].parent_id == static_cast<int>(i))
                {
                    Contour hole_contour = source_contours[j];
                    input_poly_list.push_back(internal::to_tppl_poly(hole_contour));
                }
            }

            // Execute the selected partitioning/triangulation algorithm
            TPPLPolyList result_poly_list;
            int result = 0;

            switch (algorithm)
            {
                case PartitionAlgorithm::HM:
                    result = pp.ConvexPartition_HM(&input_poly_list, &result_poly_list);
                    break;

                case PartitionAlgorithm::OPT:
                {
                    // OPT does not support holes directly. Must call RemoveHoles first.
                    TPPLPolyList no_holes_list;
                    pp.RemoveHoles(&input_poly_list, &no_holes_list);
                    // OPT works on each resulting polygon individually
                    for (auto& single_poly : no_holes_list)
                    {
                        TPPLPolyList single_result;
                        result = pp.ConvexPartition_OPT(&single_poly, &single_result);
                        if (result) result_poly_list.splice(result_poly_list.end(), single_result);
                    }
                    break;
                }

                case PartitionAlgorithm::Triangulate_EC:
                    result = pp.Triangulate_EC(&input_poly_list, &result_poly_list);
                    break;

                case PartitionAlgorithm::Triangulate_MONO:
                     result = pp.Triangulate_MONO(&input_poly_list, &result_poly_list);
                    break;
            }

            if (result == 0)
            {
                core::GlobalLogger::getCoreLogger()->error(
                    "Polypartition: Failed to partition/triangulate contours with algorithm {}",
                    static_cast<int>(algorithm));
                continue;
            }

            // Convert results back to our format and collect them
            for (const auto& poly : result_poly_list)
            {
                final_polygons.push_back(internal::from_tppl_poly(poly));
            }
        }

        return final_polygons;
    }

} // namespace cyanvne::platform::algorithm::partitioning
