/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file ww4_constants.hpp
 * @brief Common mathematical and physical constants for WAVEWATCH IV.
 * @details This header defines a set of shared constants used across the WW4
 *          model, moved from ww4_service.hpp.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI), Rahul Mahajan
 * @date 2026-04-17
 */

#ifndef WW4_UTILS_WW4_CONSTANTS_HPP
#define WW4_UTILS_WW4_CONSTANTS_HPP

/**
 * @namespace ww4_constants
 * @brief Namespace containing all physical and mathematical constants.
 * @details This namespace contains constants converted from the WAVEWATCH III
 *          (WW3) source file constants.F90.
 *          Original author in WW3: Hendrik L. Tolman.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI), Rahul Mahajan
 * @date Initial, 2026-04-09
 * @date Last update, 2026-04-17
 */
namespace ww4_constants {

// Mathematical constants
/** @brief Value of Pi. */
constexpr double PI = 3.14159265358979323846;
/** @brief 2 * Pi. */
constexpr double TPI = 2.0 * PI;
/** @brief 0.5 * Pi. */
constexpr double HPI = 0.5 * PI;
/** @brief Inverse of 2 * Pi. */
constexpr double TPIINV = 1.0 / TPI;
/** @brief Inverse of 0.5 * Pi. */
constexpr double HPIINV = 1.0 / HPI;
/** @brief Conversion factor from radians to degrees. */
constexpr double RADE = 180.0 / PI;
/** @brief Conversion factor from degrees to radians. */
constexpr double DERA = PI / 180.0;

// Physical constants
/** @brief Acceleration of gravity (m/s^2). (constants.F90) */
constexpr double GRAV = 9.806;
/** @brief Density of water (kg/m^3). (constants.F90) */
constexpr double DWAT = 1000.0;
/** @brief Density of air (kg/m^3). (constants.F90) */
constexpr double DAIR = 1.225;
/** @brief Kinematic viscosity of air (m^2/s). */
constexpr double NU_AIR = 1.4e-5;
/** @brief Kinematic viscosity of water (m^2/s). (constants.F90) */
constexpr double NU_WATER = 1.31e-6;
/** @brief Specific gravity of sediments (N.D.). */
constexpr double SED_SG = 2.65;
/** @brief von Karman's constant (N.D.). */
constexpr double KAPPA = 0.40;
/** @brief Radius of the earth (m). */
constexpr double RADIUS = 4.0e7 / TPI;

// Derived physical constants
/** @brief Inverse of gravity^2 * (2*Pi)^3. */
constexpr double G2PI3I = 1.0 / (GRAV * GRAV * TPI * TPI * TPI);
/** @brief Inverse of gravity * 2 * Pi. */
constexpr double G1PI1I = 1.0 / (GRAV * TPI);

// Model constants
/** @brief Undefined value. */
constexpr double UNDEF = -999.9;
/** @brief Minimum value for spectral density (log10). */
constexpr double ABMIN = -1.0;
/** @brief Maximum value for spectral density (log10). */
constexpr double ABMAX = 8.0;

/** @brief Maximum value for k*d in dispersion calculations. */
constexpr double KDMAX = 20.0;

/** @brief Physics factor for JONSWAP spectrum (g^2 / (2*PI)^4). */
constexpr double JONSWAP_FACTOR = 0.06175;

} // namespace ww4_constants

#endif // WW4_UTILS_WW4_CONSTANTS_HPP
