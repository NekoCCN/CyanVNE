//
// Created by Ext Culith on 2025/7/26.
//

#include "ContourTracing.h"

#include "Core/Logger/Logger.h"
#include <map>

namespace cyanvne::platform::algorithm::contourtracing
{
    ContourList trace_contour_square(
        const binarization::BinarizationResult& binarized_grid)
    {
        const auto& grid = binarized_grid.grid;
        const int width = binarized_grid.width;
        const int height = binarized_grid.height;

        // Find the first foreground point
        glm::ivec2 start_point = {-1, -1};
        bool found = false;
        for (int y = 0; y < height && !found; ++y)
        {
            for (int x = 0; x < width && !found; ++x)
            {
                if (grid[y * width + x])
                {
                    start_point = {x, y};
                    found = true;
                    break;
                }
            }
        }

        if (!found)
            return {};

        Contour contour;
        contour.points.push_back(start_point);

        glm::ivec2 current_point = start_point;
        glm::ivec2 previous_point = start_point;

        for(int i = 0; i < 4; ++i)
        {
            glm::ivec2 test_pt = start_point + internal::SQUARE_NEIGHBORS[i];

            if (test_pt.x >= 0 && test_pt.x < width && test_pt.y >= 0 && test_pt.y < height &&
                !grid[test_pt.y * width + test_pt.x])
                {
                previous_point = test_pt;
                break;
                }
        }
        int steps = 0;
        const int max_steps = width * height + 1;

        do
        {
            // Determine current direction based on the last move
            glm::ivec2 diff = current_point - previous_point;
            int current_dir = 0;
            for(int i = 0; i < 4; ++i)
            {
                if(internal::SQUARE_NEIGHBORS[i] == diff)
                {
                    current_dir = i;
                    break;
                }
            }

            for(int i = 0; i < 4; ++i)
            {
                int test_dir = (current_dir + 3 + i) % 4;
                glm::ivec2 next_point = current_point + internal::SQUARE_NEIGHBORS[test_dir];

                if (next_point.x >= 0 && next_point.x < width && next_point.y >= 0 && next_point.y < height &&
                    grid[next_point.y * width + next_point.x])
                {
                    previous_point = current_point;
                    current_point = next_point;
                    contour.points.push_back(current_point);
                    break;
                }
            }
            steps++;

        } while (current_point != start_point && steps < max_steps);

        if (contour.points.size() > 1 && contour.points.back() == start_point)
        {
            contour.points.pop_back();
        }

        return {contour};
    }

    ContourList trace_contour_moore(
        const binarization::BinarizationResult& binarized_grid)
    {
        const auto& grid = binarized_grid.grid;
        const int width = binarized_grid.width;
        const int height = binarized_grid.height;

        glm::ivec2 start_point = {-1, -1};
        bool found = false;
        for (int y = 0; y < height && !found; ++y)
        {
            for (int x = 0; x < width && !found; ++x)
            {
                if (grid[y * width + x])
                {
                    start_point = {x, y};
                    found = true;
                    break;
                }
            }
        }

        if (!found) return {};

        Contour contour;
        contour.points.push_back(start_point);

        glm::ivec2 current_point = start_point;
        glm::ivec2 backtrack_point = {start_point.x, start_point.y + 1}; // Assume we enter from top

        // State for Jacob's stopping criterion
        const glm::ivec2 jacob_start_point = start_point;
        const glm::ivec2 jacob_backtrack_point = backtrack_point;
        bool jacob_condition_met = false;
        int steps = 0;
        const int max_steps = width * height + 1;

        do
        {
            int start_dir = 0;
            for (int i = 0; i < 8; ++i)
            {
                if (current_point + internal::MOORE_NEIGHBORS[i] == backtrack_point)
                {
                    start_dir = (i + 1) % 8;
                    break;
                }
            }

            glm::ivec2 next_point = {-1, -1};
            for (int i = 0; i < 8; ++i)
            {
                int current_dir = (start_dir + i) % 8;
                glm::ivec2 test_point = current_point + internal::MOORE_NEIGHBORS[current_dir];

                if (test_point.x >= 0 && test_point.x < width && test_point.y >= 0 && test_point.y < height &&
                    grid[test_point.y * width + test_point.x])
                {
                    next_point = test_point;
                    break;
                }
            }

            if (next_point.x == -1) break;

            backtrack_point = current_point;
            current_point = next_point;

            if (current_point == jacob_start_point && backtrack_point == jacob_backtrack_point)
            {
                jacob_condition_met = true;
            }
            else
            {
                contour.points.push_back(current_point);
            }
            steps++;

        } while (!jacob_condition_met && steps < max_steps);

        return {contour};
    }

    ContourList trace_contour_pavlidis(const binarization::BinarizationResult& binarized_grid)
     {
        const auto& grid = binarized_grid.grid;
        const int width = binarized_grid.width;
        const int height = binarized_grid.height;

        // Corrected start point search: bottom-most, then left-most pixel
        glm::ivec2 start_point = {-1, -1};
        for (int y = height - 1; y >= 0; --y)
        {
            for (int x = 0; x < width; ++x)
            {
                if (grid[y * width + x])
                {
                    start_point = {x, y};
                    goto found_pavlidis_start;
                }
            }
        }

        // Goto label to exit the loop early
        found_pavlidis_start:;

        if (start_point.x == -1) return {};

        Contour contour;
        contour.points.push_back(start_point);

        glm::ivec2 p = start_point;
        // Initial direction is West, as we start from the bottom-left
        int dir = 3;

        int steps = 0;
        const int max_steps = width * height + 1;

        do
        {
            // Define p1, p2, p3 relative to the current direction
            const glm::ivec2& fwd = internal::SQUARE_NEIGHBORS[dir];
            const glm::ivec2& left = internal::SQUARE_NEIGHBORS[(dir + 3) % 4];

            glm::ivec2 p1 = p + left; // Left
            glm::ivec2 p2 = p + left + fwd; // Front-Left
            glm::ivec2 p3 = p + fwd; // Front

            auto is_pixel_fg = [&](const glm::ivec2& pt)
            {
                return pt.x >= 0 && pt.x < width && pt.y >= 0 && pt.y < height && grid[pt.y * width + pt.x];
            };

            // Corrected predictive logic
            if (is_pixel_fg(p2))
            {
                p = p2;
                dir = (dir + 3) % 4; // Turn left
            }
            else if (is_pixel_fg(p3))
            {
                p = p3; // Go straight
            }
            else if (is_pixel_fg(p1))
            {
                 p = p1; // Go left (should be less common)
            }
            else
            {
                dir = (dir + 1) % 4; // Turn right in place
            }

            if (p != start_point)
            {
                contour.points.push_back(p);
            }
            steps++;

        } while (p != start_point && steps < max_steps);

        return {contour};
    }

    ContourList trace_contours_suzuki(const binarization::BinarizationResult& binarized_grid)
    {
        const int width = binarized_grid.width;
        const int height = binarized_grid.height;

        if (width == 0 || height == 0)
        {
            return {};
        }

        std::vector<int> image(width * height);
        for (size_t i = 0; i < binarized_grid.grid.size(); ++i)
        {
            image[i] = binarized_grid.grid[i] ? 1 : 0;
        }

        ContourList contours;
        int nbd = 1; // Border counter starts at 2

        for (int y = 0; y < height; ++y)
        {
            int lnbd = 1;
            for (int x = 0; x < width; ++x)
            {
                const int current_idx = y * width + x;
                if (image[current_idx] == 0) continue;

                const int p1_val = (x > 0) ? image[current_idx - 1] : 0;

                bool is_outer_border_start = (image[current_idx] == 1 && p1_val == 0);
                bool is_hole_border_start = (image[current_idx] == 1 && p1_val > 1);

                if (is_outer_border_start || is_hole_border_start)
                {
                    nbd++;
                    glm::ivec2 start_point = {x, y};

                    Contour contour;
                    contour.is_hole = is_hole_border_start;
                    contour.parent_id = is_outer_border_start ? lnbd : p1_val;

                    contours.push_back(contour);
                    Contour& new_contour = contours.back();

                    glm::ivec2 current_point = start_point;
                    glm::ivec2 backtrack_point = {x - 1, y};

                    // Full tracing and marking loop
                    do
                    {
                        new_contour.points.push_back(current_point);
                        image[current_point.y * width + current_point.x] = nbd;

                        int start_dir = 0;
                        for (int i = 0; i < 8; ++i)
                        {
                            if (current_point + internal::MOORE_NEIGHBORS[i] == backtrack_point)
                            {
                                start_dir = (i + 1) % 8;
                                break;
                            }
                        }

                        glm::ivec2 next_point = {-1, -1};
                        for (int i = 0; i < 8; ++i)
                        {
                            int current_dir = (start_dir + i) % 8;
                            glm::ivec2 test_point = current_point + internal::MOORE_NEIGHBORS[current_dir];

                            if (test_point.x >= 0 && test_point.x < width && test_point.y >= 0 && test_point.y < height &&
                                image[test_point.y * width + test_point.x] != 0)
                            {
                                next_point = test_point;
                                break;
                            }
                        }

                        if (next_point.x == -1)
                            break;

                        backtrack_point = current_point;
                        current_point = next_point;

                    } while (current_point != start_point);
                }

                // Corrected lnbd update logic
                if (image[current_idx] != 0)
                {
                    lnbd = std::abs(image[current_idx]);
                }
            }
        }

        // Corrected parent ID transformation logic
        std::map<int, int> nbd_to_idx;
        for(size_t i = 0; i < contours.size(); ++i)
        {
            const auto& c = contours[i];
            if (!c.points.empty())
            {
                // The NBD value was stored in the image grid at the contour's points
                int nbd_val = image[c.points[0].y * width + c.points[0].x];
                nbd_to_idx[nbd_val] = i;
            }
        }

        for(auto& contour : contours)
        {
            if (contour.parent_id > 1)
            {
                if (nbd_to_idx.count(contour.parent_id))
                {
                    contour.parent_id = nbd_to_idx.at(contour.parent_id);
                }
                else
                {
                    contour.parent_id = -1; // Should not happen in a correct implementation
                }
            }
            else
            {
                contour.parent_id = -1; // Top-level contour
            }
        }

        return contours;
    }

    ContourList trace_contours(const binarization::BinarizationResult& binarized_grid,
        ContourTracingAlgorithm algorithm)
    {
        switch (algorithm)
        {
        case ContourTracingAlgorithm::Square:
            return trace_contour_square(binarized_grid);
        case ContourTracingAlgorithm::Moore:
            return trace_contour_moore(binarized_grid);
        case ContourTracingAlgorithm::Pavlidis:
            return trace_contour_pavlidis(binarized_grid);
        case ContourTracingAlgorithm::Suzuki:
            return trace_contours_suzuki(binarized_grid);
        default:
            cyanvne::core::GlobalLogger::getCoreLogger()->error("Unknown contour tracing algorithm: {}", static_cast<int>(algorithm));
            return {};
        }
    }
}
