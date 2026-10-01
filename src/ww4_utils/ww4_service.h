/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file ww4_service.h
 * @brief Common mathematical and physical constants for WAVEWATCH IV.
 * @details This header defines a set of shared constants used across the WW4
 *          model.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-04-09
 * @date Last update : 2026-10-01
 */

#pragma once

#include "ww4_utils/ww4_constants.h"
#include <cmath>

namespace ww4_utils {

// --- Dispersion -------------------------------------------------------------
/**
 * @struct Dispersion
 * @brief Structure to hold wave dispersion parameters.
 * @details This structure contains the wavenumber and group velocity
 *          calculated from the dispersion relation.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-04-09
 * @date Last update : 2026-09-28
 * @var Dispersion::k
 * @brief Wavenumber (rad/m).
 * @var Dispersion::cg
 * @brief Group velocity (m/s).
 */
struct Dispersion {
  double k;
  double cg;
};

namespace ww4_service {

// --- wavenumber_Beji --------------------------------------------------------
/**
 * @brief Calculate wavenumber and group velocity using Beji (2013).
 * @param omega Intrinsic frequency (rad/s).
 * @param h Water depth (m).
 * @return Dispersion struct containing k and cg.
 */
Dispersion wavenumber_Beji(double omega, double h);

// --- JONSWAP_5p -------------------------------------------------------------
/**
 * @brief Calculate 5-parameter JONSWAP spectrum.
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

// --- dist_Haversine ---------------------------------------------------------
/**
 * @brief Calculate the haversine distance between two points on a sphere.
 * @param lon1 Longitude of 1st point (degrees).
 * @param lat1 Latitude of 1st point (degrees).
 * @param lon2 Longitude of 2nd point (degrees).
 * @param lat2 Latitude of 2nd point (degrees).
 * @return Spherical distance (radians).
 */
double dist_Haversine(double lon1, double lat1, double lon2, double lat2);

// --- dist_on_sphere ---------------------------------------------------------
/**
 * @brief Calculate the spherical distance between two points in meters.
 * @param lon1 Longitude of 1st point (degrees).
 * @param lat1 Latitude of 1st point (degrees).
 * @param lon2 Longitude of 2nd point (degrees).
 * @param lat2 Latitude of 2nd point (degrees).
 * @return Spherical distance (meters).
 */
double dist_on_sphere(double lon1, double lat1, double lon2, double lat2);

} // namespace ww4_service

} // namespace ww4_utils
