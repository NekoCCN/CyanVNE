//
// Created by Ext Culith on 2025/7/26.
//

#include "Simplification.h"
#include <glm/glm.hpp>
#include <vector>
#include <queue>
#include <set>

namespace cyanvne::platform::algorithm::simplification
{
    namespace internal
    {
        float perpendicular_distance(const glm::vec2& p, const glm::vec2& start, const glm::vec2& end)
        {
            float dx = end.x - start.x;
            float dy = end.y - start.y;
            float mag_sq = dx * dx + dy * dy;
            if (mag_sq == 0.0f) return glm::distance(p, start);
            float t = glm::clamp(((p.x - start.x) * dx + (p.y - start.y) * dy) / mag_sq, 0.0f, 1.0f);
            glm::vec2 projection = start + t * glm::vec2(dx, dy);
            return glm::distance(p, projection);
        }

        void rdp_recursive(const std::vector<glm::vec2>& points, float epsilon, std::vector<bool>& markers, int start_index, int end_index)
        {
            if (start_index >= end_index) return;
            float max_dist = 0.0f;
            int max_index = -1;
            for (int i = start_index + 1; i < end_index; ++i)
            {
                float dist = perpendicular_distance(points[i], points[start_index], points[end_index]);
                if (dist > max_dist)
                {
                    max_dist = dist;
                    max_index = i;
                }
            }
            if (max_dist > epsilon && max_index != -1)
            {
                markers[max_index] = true;
                rdp_recursive(points, epsilon, markers, start_index, max_index);
                rdp_recursive(points, epsilon, markers, max_index, end_index);
            }
        }

        struct VWPoint
        {
            glm::vec2 p;
            float effective_area = std::numeric_limits<float>::max();
            VWPoint *prev = nullptr;
            VWPoint *next = nullptr;
            bool active = true;
        };

        struct CompareVWPointForSet
        {
            bool operator()(const VWPoint* a, const VWPoint* b) const
            {
                if (a->effective_area != b->effective_area)
                {
                    return a->effective_area < b->effective_area;
                }

                return a < b;
            }
        };

        float triangle_area(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c)
        {
            return 0.5f * std::abs(a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y));
        }

    } // namespace internal

    Contour simplify_contour_rdp(const Contour& source_contour, float epsilon)
    {
        if (source_contour.points.size() < 3) return source_contour;

        std::vector<glm::vec2> float_points;
        float_points.reserve(source_contour.points.size());
        for(const auto& p : source_contour.points) float_points.push_back(glm::vec2(p));

        std::vector<bool> markers(float_points.size(), false);
        markers.front() = true;
        markers.back() = true;

        internal::rdp_recursive(float_points, epsilon, markers, 0, float_points.size() - 1);

        Contour simplified_contour;
        simplified_contour.is_hole = source_contour.is_hole;
        simplified_contour.parent_id = source_contour.parent_id;

        for (size_t i = 0; i < float_points.size(); ++i)
        {
            if (markers[i])
            {
                simplified_contour.points.push_back(glm::ivec2(glm::round(float_points[i])));
            }
        }

        return simplified_contour;
    }

    Contour simplify_contour_vw(const Contour& source_contour, float epsilon)
    {
        if (source_contour.points.size() < 3)
            return source_contour;

        std::vector<internal::VWPoint> vw_points(source_contour.points.size());
        for (size_t i = 0; i < source_contour.points.size(); ++i)
        {
            vw_points[i].p = glm::vec2(source_contour.points[i]);
            vw_points[i].prev = &vw_points[(i == 0) ? vw_points.size() - 1 : i - 1];
            vw_points[i].next = &vw_points[(i == vw_points.size() - 1) ? 0 : i + 1];
        }

        std::set<internal::VWPoint*, internal::CompareVWPointForSet> pq;
        for (size_t i = 0; i < vw_points.size(); ++i)
        {
            if (i == 0 || i == vw_points.size() - 1)
                continue;

            vw_points[i].effective_area = internal::triangle_area(vw_points[i].prev->p, vw_points[i].p, vw_points[i].next->p);
            pq.insert(&vw_points[i]);
        }

        while (!pq.empty())
        {
            internal::VWPoint* min_point = *pq.begin();

            if (min_point->effective_area >= epsilon)
            {
                break;
            }

            pq.erase(pq.begin());

            internal::VWPoint* prev_p = min_point->prev;
            internal::VWPoint* next_p = min_point->next;

            min_point->active = false;
            prev_p->next = next_p;
            next_p->prev = prev_p;

            auto update_neighbor = [&](internal::VWPoint* neighbor)
            {
                if (neighbor->prev && neighbor->next && neighbor->active &&
                    neighbor->prev != neighbor->next)
                {
                    pq.erase(neighbor);
                    neighbor->effective_area = internal::triangle_area(neighbor->prev->p, neighbor->p, neighbor->next->p);
                    pq.insert(neighbor);
                }
            };

            update_neighbor(prev_p);
            update_neighbor(next_p);
        }

        Contour simplified_contour;
        simplified_contour.is_hole = source_contour.is_hole;
        simplified_contour.parent_id = source_contour.parent_id;

        for(const auto& p : vw_points)
        {
            if (p.active)
            {
                simplified_contour.points.push_back(glm::ivec2(glm::round(p.p)));
            }
        }

        return simplified_contour;
    }

    Contour simplify_contour(
        const Contour& source_contour,
        SimplificationAlgorithm algorithm,
        float epsilon)
    {
        switch (algorithm)
        {
            case SimplificationAlgorithm::RDP:
                return simplify_contour_rdp(source_contour, epsilon);
            case SimplificationAlgorithm::VisvalingamWhyatt:
                return simplify_contour_vw(source_contour, epsilon);
            default:
                return source_contour;
        }
    }

    ContourList simplify_contours(
        const ContourList& source_contours,
        SimplificationAlgorithm algorithm,
        float epsilon)
    {
        ContourList simplified_list;
        simplified_list.reserve(source_contours.size());
        for (const auto& contour : source_contours)
        {
            simplified_list.push_back(simplify_contour(contour, algorithm, epsilon));
        }
        return simplified_list;
    }

} // namespace cyanvne::platform::algorithm::simplification
