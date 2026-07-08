/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file ww4_service.cpp
 * @brief Common mathematical and physical constants for WAVEWATCH IV
 *        and various service routines.
 * @details This source file holds subroutine codes. The constants
 *          are implemented as constexpr in ww4_constants.h.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-04-10
 * @date Last update : 2026-07-07
 */

#include "ww4_utils/ww4_service.h"

/**
 * @namespace ww4_constants
 * @brief Namespace containing all physical and mathematical constants.
 * @details This namespace contains constants converted from the WAVEWATCH III
 *          (WW3) source file constants.F90.
 *          Original author in WW3: Hendrik L. Tolman.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI), Rahul Mahajan
 * @date Initial, 2026-04-09
 * @date Last update : 2026-07-07
 * @var Radians2Degrees
 * @brief Conversion factor from radians to degrees.
 * @var Degrees2Radians
 * @brief Conversion factor from degrees to radians.
 * @var GRAV
 * @brief Acceleration of gravity (m/s^2). (constants.F90)
 * @var DWAT
 * @brief Density of water (kg/m^3). (constants.F90)
 * @var DAIR
 * @brief Density of air (kg/m^3). (constants.F90)
 * @var NU_AIR
 * @brief Kinematic viscosity of air (m^2/s).
 * @var NU_WATER
 * @brief Kinematic viscosity of water (m^2/s). (constants.F90)
 * @var SED_SG
 * @brief Specific gravity of sediments (N.D.).
 * @var KAPPA
 * @brief von Karman's constant (N.D.).
 * @var RADIUS
 * @brief Radius of the earth (m).
 * @var UNDEF
 * @brief Undefined value.
 * @var ABMIN
 * @brief Minimum value for spectral density (log10).
 * @var ABMAX
 * @brief Maximum value for spectral density (log10).
 * @var KDMAX
 * @brief Maximum value for k*d in dispersion calculations.
 * @var JONSWAP_FACTOR
 * @brief Physics factor for JONSWAP spectrum (g^2 / (2*PI)^4).
 */

/**
 * @namespace ww4_utils
 * @brief Utilities for WAVEWATCH IV.
 */
namespace ww4_utils {

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
 * @date 2026-05-21
 * @param omega Intrinsic frequency (rad/s).
 * @param h Water depth (m).
 * @return Dispersion struct containing k and cg.
 */
Dispersion wavenumber_Beji(double omega, double h) {
  if (h <= 0.0 || omega <= 0.0) {
    return {0.0, 0.0};
  }

  // Deep water KH0 = omega^2 * h / g
  double kh0 = (omega * omega * h) / ww4_constants::GRAV;

  // Intermediate term for Beji's improved Eckart formula
  double tmp = 1.55 + 1.3 * kh0 + 0.216 * kh0 * kh0;

  // Calculate KH using the approximation
  double kh =
      kh0 *
      (1.0 + std::pow(kh0, 1.09) *
                 (1.0 / std::exp(std::min(ww4_constants::KDMAX, tmp)))) /
      std::sqrt(std::tanh(std::min(ww4_constants::KDMAX, kh0)));

  double k = kh / h;

  // Group velocity calculation from linear wave theory
  double cg =
      0.5 *
      (1.0 + (2.0 * kh / std::sinh(std::min(ww4_constants::KDMAX, 2.0 * kh)))) *
      omega / k;

  return {k, cg};
}

/**
 * @brief Calculate 5-parameter JONSWAP spectrum.
 * @details Ported from WW3 routine EJ5P in w3srcemd.f90.
 *          Original author in WW3: G. Ph. van Vledder.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date 2026-05-21
 * @param f Frequency (Hz).
 * @param fp Peak frequency (Hz).
 * @param alpha Phillip's constant.
 * @param gamma Peak enhancement factor.
 * @param siga Sigma_a (for f <= fp).
 * @param sigb Sigma_b (for f > fp).
 * @return Spectral density E(f).
 */
double JONSWAP_5p(double f, double fp, double alpha, double gamma, double siga,
                  double sigb) {
  if (f <= 0.0 || fp <= 0.0) {
    return 0.0;
  }

  double sigma = (f <= fp) ? siga : sigb;
  double f_ratio = fp / f;
  double r = std::exp(-0.5 * std::pow((f - fp) / (sigma * fp), 2));

  return ww4_constants::JONSWAP_FACTOR * alpha * std::pow(f, -5) *
         std::exp(-1.25 * std::pow(f_ratio, 4)) * std::pow(gamma, r);
}

/**
 * @brief Calculate the haversine distance between two points on a sphere.
 * @details Ported from WW3 routine DIST_HAVERSINE in w3servmd.F90.
 *          Method: R.W. Sinnott, "Virtues of the Haversine",
 *                  Sky and Telescope, vol. 68, no. 2, 1984, p. 159.
 *          Original author in WW3: D. A. Honegger (USACE/ERDC).
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date 2026-05-21
 * @param lon1 Longitude of 1st point (degrees).
 * @param lat1 Latitude of 1st point (degrees).
 * @param lon2 Longitude of 2nd point (degrees).
 * @param lat2 Latitude of 2nd point (degrees).
 * @return Spherical distance (radians).
 */
double dist_Haversine(double lon1, double lat1, double lon2, double lat2) {
  // Compute differences in latitude and longitude in radians
  double dlat = (lat2 - lat1) * ww4_constants::Degrees2Radians;
  double dlon = (lon2 - lon1) * ww4_constants::Degrees2Radians;

  // Compute the haversine of the central angle
  double a = std::pow(std::sin(dlat / 2.0), 2) +
             std::cos(lat1 * ww4_constants::Degrees2Radians) *
                 std::cos(lat2 * ww4_constants::Degrees2Radians) *
                 std::pow(std::sin(dlon / 2.0), 2);

  // Compute the angular distance (c), ensuring no precision issues
  double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(std::max(0.0, 1.0 - a)));

  // Compute the spherical distance in radians
  return c;
}

/**
 * @brief Calculate the spherical distance between two points in meters.
 * @details Ported from WW3 routine DIST_SPHERE in w3srcemd.f90.
 *          Uses the haversine formula for computation.
 *          Original author in WW3: Fabrice Ardhuin.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date 2026-05-21
 * @param lon1 Longitude of 1st point (degrees).
 * @param lat1 Latitude of 1st point (degrees).
 * @param lon2 Longitude of 2nd point (degrees).
 * @param lat2 Latitude of 2nd point (degrees).
 * @return Spherical distance (meters).
 */
double dist_on_sphere(double lon1, double lat1, double lon2, double lat2) {
  // Compute the haversine distance in radians
  double dist_rad = dist_Haversine(lon1, lat1, lon2, lat2);

  // Convert to meters using the radius of the earth
  return dist_rad * ww4_constants::RADIUS;
}

} // namespace ww4_service

} // namespace ww4_utils
