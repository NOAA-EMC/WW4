/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file ww4_service.hpp
 * @brief Common mathematical and physical constants for WAVEWATCH IV.
 * @details This header defines a set of shared constants used across the WW4
 *          model.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-04-09
 * @date Last update, 2026-04-17
 */

#ifndef WW4_UTILS_WW4_SERVICE_HPP
#define WW4_UTILS_WW4_SERVICE_HPP

#include "ww4_utils/ww4_constants.hpp"
#include <cmath>

namespace ww4_utils {

/**
 * @struct Dispersion
 * @brief Structure to hold wave dispersion parameters.
 * @details This structure contains the wavenumber and group velocity
 *          calculated from the dispersion relation.
 */
struct Dispersion {
  double k;  /**< Wavenumber (rad/m). */
  double cg; /**< Group velocity (m/s). */
};

/**
 * @namespace ww4_service
 * @brief Namespace providing utility mathematical routines.
 * @details This namespace contains ported routines from WAVEWATCH III (WW3).
 */
namespace ww4_service {

/**
 * @brief Calculate wavenumber and group velocity using Beji (2013).
 * @details Ported from WW3 routine WAVNU3 in w3dispmd.ftn.
 *          Calculates wavenumber (k) and group velocity (cg) using the
 *          improved Eckart formula by Beji (2013).
 *          Original author in WW3: Aron Roland.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date 2026-04-13
 * @param omega Intrinsic frequency (rad/s).
 * @param h Water depth (m).
 * @return Dispersion struct containing k and cg.
 */
Dispersion wavenumber_Beji(double omega, double h);

/**
 * @brief Calculate 5-parameter JONSWAP spectrum.
 * @details Ported from WW3 routine EJ5P in w3srcemd.f90.
 *          Original author in WW3: G. Ph. van Vledder.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date 2026-04-10
 * @param f Frequency (Hz).
 * @param fp Peak frequency (Hz).
 * @param alpha Phillip's constant.
 * @param gamma Peak enhancement factor.
 * @param siga Sigma_a (for f <= fp).
 * @param sigb Sigma_b (for f > fp).
 * @return Spectral density E(f).
 */
double JONSWAP_5p(double f, double fp, double alpha, double gamma, double siga,
                  double sigb);

/**
 * @brief Calculate the haversine distance between two points on a sphere.
 * @details Ported from WW3 routine DIST_HAVERSINE in w3servmd.F90.
 *          Method: R.W. Sinnott, "Virtues of the Haversine",
 *                  Sky and Telescope, vol. 68, no. 2, 1984, p. 159.
 *          Original author in WW3: D. A. Honegger (USACE/ERDC).
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date 2026-04-10
 * @param lon1 Longitude of 1st point (degrees).
 * @param lat1 Latitude of 1st point (degrees).
 * @param lon2 Longitude of 2nd point (degrees).
 * @param lat2 Latitude of 2nd point (degrees).
 * @return Spherical distance (radians).
 */
double dist_Haversine(double lon1, double lat1, double lon2, double lat2);

/**
 * @brief Calculate the spherical distance between two points in meters.
 * @details Ported from WW3 routine DIST_SPHERE in w3srcemd.f90.
 *          Uses the haversine formula for computation.
 *          Original author in WW3: Fabrice Ardhuin.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date 2026-04-10
 * @param lon1 Longitude of 1st point (degrees).
 * @param lat1 Latitude of 1st point (degrees).
 * @param lon2 Longitude of 2nd point (degrees).
 * @param lat2 Latitude of 2nd point (degrees).
 * @return Spherical distance (meters).
 */
double dist_on_sphere(double lon1, double lat1, double lon2, double lat2);

} // namespace ww4_service

} // namespace ww4_utils

#endif // WW4_UTILS_WW4_SERVICE_HPP
