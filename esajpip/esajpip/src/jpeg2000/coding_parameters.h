#ifndef _JPEG2000_CODING_PARAMETERS_H_
#define _JPEG2000_CODING_PARAMETERS_H_

#include <vector>
#include <cmath>

#include "point.h"
#include "trace.h"
#include "packet.h"

namespace jpeg2000 {
    /**
     * Contains the coding parameters of a JPEG2000 image codestream.
     * This class can be serialized and printed.
     */
    class CodingParameters {
    private:
        /**
         * Contains the number of precincts of each
         * resolution level.
         */
        vector<int> total_precincts;

        /**
         * Returns the index of a packet according to the RPCL progression.
         * @param l Quality layer.
         * @param r Resolution level.
         * @param c Component.
         * @param px Precinct position X.
         * @param py Precinct position Y.
         */
        int GetProgressionIndexRPCL(int l, int r, int c, int px, int py) const {
            Size precinct_point = GetPrecincts(r, size);
            return (total_precincts[r] * num_components * num_layers) +
                   (py * precinct_point.x * num_components * num_layers) +
                   (px * num_components * num_layers) + (c * num_layers) + l;
        }

        /**
         * Returns the index of a packet according to the RLCP progression.
         * @param l Quality layer.
         * @param r Resolution level.
         * @param c Component.
         * @param px Precinct position X.
         * @param py Precinct position Y.
         */
        int GetProgressionIndexRLCP(int l, int r, int c, int px, int py) const {
            Size precinct_point = GetPrecincts(r, size);
            return (total_precincts[r] * num_components * num_layers) +
                   (l * num_components * precinct_point.x * precinct_point.y) +
                   (c * precinct_point.x * precinct_point.y) + (py * precinct_point.x) + px;
        }

        /**
         * Returns the index of a packet according to the LRCP progression.
         * @param l Quality layer.
         * @param r Resolution level.
         * @param c Component.
         * @param px Precinct position X.
         * @param py Precinct position Y.
         */
        int GetProgressionIndexLRCP(int l, int r, int c, int px, int py) const {
            Size precinct_point = GetPrecincts(r, size);
            return (l * total_precincts[num_levels + 1] * num_components) + (num_components * total_precincts[r]) +
                   (c * precinct_point.x * precinct_point.y) + (py * precinct_point.x) + px;
        }

    public:
        Size size;                ///< Image size
        int num_levels;            ///< Number of resolution levels
        int num_layers;            ///< Number of quality layers
        int progression;        ///< Progression order
        int num_components;        ///< Number of components

        /**
         * Precinct sizes of each resolution level.
         */
        vector<Size> precinct_size;

        /**
         * All the progression orders defined in the JPEG2000
         * standard (Part 1).
         */
        enum {
            LRCP_PROGRESSION = 0,        ///< LRCP
            RLCP_PROGRESSION = 1,        ///< RLCP
            RPCL_PROGRESSION = 2,        ///< RPCL
            PCRL_PROGRESSION = 3,        ///< PCRL
            CPRL_PROGRESSION = 4        ///< CPRL
        };

        /**
         * Initializes the object.
         */
        CodingParameters() {
            num_levels = 0;
            num_layers = 0;
            progression = 0;
            num_components = 0;
        }

        /**
         * Copy constructor.
         */
        CodingParameters(const CodingParameters &cod_params) {
            *this = cod_params;
        }

        /**
         * Fills the vector <code>total_precincts</code>.
         */
        void FillTotalPrecinctsVector();

        /**
         * Copy assignment.
         */
        CodingParameters &operator=(const CodingParameters &cod_params) {
            size = cod_params.size;
            num_levels = cod_params.num_levels;
            num_layers = cod_params.num_layers;
            progression = cod_params.progression;
            num_components = cod_params.num_components;
            precinct_size = cod_params.precinct_size;
            total_precincts = cod_params.total_precincts;
            return *this;
        }

        friend ostream &operator<<(ostream &out, const CodingParameters &params) {
            out << "Progression: " <<
                (params.progression == LRCP_PROGRESSION ? "LRCP" :
                 (params.progression == RLCP_PROGRESSION ? "RLCP" :
                  (params.progression == RPCL_PROGRESSION ? "RPCL" :
                   (params.progression == PCRL_PROGRESSION ? "PCRL" :
                    (params.progression == CPRL_PROGRESSION ? "CPRL" : "UNKOWN"))))) << endl
                << "Size: " << params.size << endl << "Num. of levels: " << params.num_levels << endl
                << "Num. of layers: " << params.num_layers << endl
                << "Num. of components: " << params.num_components << endl << "Precinct size: { ";
            for (size_t i = 0; i < params.precinct_size.size(); ++i)
                out << params.precinct_size[i] << " ";
            out << "}" << endl;
            return out;
        }

        /**
         * Returns <code>true</code> if the progression is RLCP or RPCL.
         */
        bool IsResolutionProgression() const {
            return progression == RLCP_PROGRESSION || progression == RPCL_PROGRESSION;
        }

        /**
         * Returns a precinct coordinate adjusted to a given resolution level.
         * @param r Resolution level.
         * @param point Precinct coordinate.
         */
        Size GetPrecincts(int r, const Size &point) const {
            return Size(
                    (int) ceil(ceil((double) point.x / (1L << (num_levels - r))) / (double) precinct_size[r].x),
                    (int) ceil(ceil((double) point.y / (1L << (num_levels - r))) / (double) precinct_size[r].y)
            );
        }

        /**
         * Returns the index of a packet according to the
         * progression order.
         * @param packet Packet information.
         */
        int GetProgressionIndex(const Packet &packet) const {
            //if (total_precincts.empty())
            //    FillTotalPrecinctsVector();
            switch (progression) {
                case LRCP_PROGRESSION:
                    return GetProgressionIndexLRCP(packet.layer, packet.resolution, packet.component, packet.precinct_xy.x, packet.precinct_xy.y);
                case RLCP_PROGRESSION:
                    return GetProgressionIndexRLCP(packet.layer, packet.resolution, packet.component, packet.precinct_xy.x, packet.precinct_xy.y);
                case RPCL_PROGRESSION:
                    return GetProgressionIndexRPCL(packet.layer, packet.resolution, packet.component, packet.precinct_xy.x, packet.precinct_xy.y);
                default:
                    ERROR("Progression (" << progression << ") not supported");
            }
            return 0;
        }

        /**
         * Returns the data-bin identifier associated to the
         * given packet.
         * @param packet Packet information.
         */
        int GetPrecinctDataBinId(const Packet &packet) const {
            //if (total_precincts.empty())
            //    FillTotalPrecinctsVector();
            Size precinct_point = GetPrecincts(packet.resolution, size);
            int s = total_precincts[packet.resolution] + (precinct_point.x * packet.precinct_xy.y) +
                    packet.precinct_xy.x;
            return (packet.component + (s * num_components));
        }

        /**
         * Returns the resolution level according to the given size and
         * the closest round policy.
         * @param res_size Resolution size.
         * @param res_image_size Image size associated to the
         * resolution level returned.
         */
        int GetClosestResolution(const Size &res_size, Size *res_image_size) const;

        /**
         * Returns the resolution level according to the given size and
         * the round-up round policy.
         * @param res_size Resolution size.
         * @param res_image_size Image size associated to the
         * resolution level returned.
         */
        int GetRoundUpResolution(const Size &res_size, Size *res_image_size) const;

        /**
         * Returns the resolution level according to the given size and
         * the round-down round policy.
         * @param res_size Resolution size.
         * @param res_image_size Image size associated to the
         * resolution level returned.
         */
        int GetRoundDownResolution(const Size &res_size, Size *res_image_size) const;

        virtual ~CodingParameters() {
        }
    };
}

#endif /* _JPEG2000_CODING_PARAMETERS_H_ */
