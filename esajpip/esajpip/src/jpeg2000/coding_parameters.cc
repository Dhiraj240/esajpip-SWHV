#include "coding_parameters.h"

namespace jpeg2000 {

    void CodingParameters::FillTotalPrecinctsVector() {
        int pa = 0;
        Size precinct_point;

        total_precincts.clear();
        total_precincts.push_back(pa);

        for (int i = 0; i <= num_levels; ++i) {
            precinct_point = GetPrecincts(i, size);
            pa += precinct_point.x * precinct_point.y;
            total_precincts.push_back(pa);
        }
    }

    int CodingParameters::GetClosestResolution(const Size &res_size, Size *res_image_size) const {
        int distance, final_r = 0;
        int distance_x = size.x - res_size.x;
        int distance_y = size.y - res_size.y;
        int min = abs(distance_x) + abs(distance_y);
        int res_image_x, res_image_y;
        res_image_size->x = size.x;
        res_image_size->y = size.y;

        for (int r = 1; r <= num_levels; ++r) {
            res_image_x = (int) ceil((double) size.x / (1L << r));
            res_image_y = (int) ceil((double) size.y / (1L << r));
            distance_x = res_image_x - res_size.x;
            distance_y = res_image_y - res_size.y;
            distance = abs(distance_x) + abs(distance_y);

            if (distance < min) {
                res_image_size->x = res_image_x;
                res_image_size->y = res_image_y;
                min = distance;
                final_r = r;
            }
        }

        int res = (num_levels - final_r);

        if (res > num_levels) res = num_levels;
        else if (res < 0) res = 0;

        return res;
    }

    int CodingParameters::GetRoundUpResolution(const Size &res_size, Size *res_image_size) const {
        int r = num_levels;
        bool bigger = false;

        while (!bigger && r >= 0) {
            res_image_size->x = (int) ceil((double) size.x / (1L << r));
            res_image_size->y = (int) ceil((double) size.y / (1L << r));
            if ((res_image_size->x >= res_size.x) &&
                (res_image_size->y >= res_size.y))
                bigger = true;
            else r--;
        }

        int res = (num_levels - r);

        if (res > num_levels) res = num_levels;
        else if (res < 0) res = 0;

        return res;
    }

    int CodingParameters::GetRoundDownResolution(const Size &res_size, Size *res_image_size) const {
        int r = 0;
        bool smaller = false;

        while (!smaller && r <= num_levels) {
            res_image_size->x = (int) ceil((double) size.x / (1L << r));
            res_image_size->y = (int) ceil((double) size.y / (1L << r));
            if ((res_image_size->x <= res_size.x) &&
                (res_image_size->y <= res_size.y))
                smaller = true;
            else r++;
        }

        int res = (num_levels - r);

        if (res > num_levels) res = num_levels;
        else if (res < 0) res = 0;

        return res;
    }
}
