/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file w4core_wave.h
 * @brief Time stepping routine for the WAVEWATCH IV core.
 * @details This header defines the time stepping routine for the WW4 core.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-04-03
 * @date Last update : 2026-09-25
 * @note The architectural design of this routine follows the structure of
 *       the multi-grid shell (ww3_multi.F90) in WAVEWATCH III.
 *       Original author of WW3 multi-grid shell: Hendrik L. Tolman.
 */

#pragma once

#include "ww4_utils/time_management.h"
#include "ww4_utils/ww4_input_utils.h"
#include <iostream>
#include <optional>

namespace ww4_core {

// --- Solver Sub-Routines ----------------------------------------------------
/**
 * @brief Solver routine for Ultimate Quickest (UQ) scheme on a regular grid.
 * @param timeStep Time step in seconds.
 * @param os Output stream.
 */
void w4core_wave_uq(double timeStep, std::ostream &os = std::cout);

/**
 * @brief Solver routine for triangular unstructured grid approach.
 * @param timeStep Time step in seconds.
 * @param os Output stream.
 */
void w4core_wave_triangular(double timeStep, std::ostream &os = std::cout);

/**
 * @brief Solver routine for Spherical Multiple-Cell (SMC) grid solver.
 * @param timeStep Time step in seconds.
 * @param os Output stream.
 */
void w4core_wave_smc(double timeStep, std::ostream &os = std::cout);

// --- w4core_wave ------------------------------------------------------------
/**
 * @brief Time stepping routine for the WAVEWATCH IV core.
 * @details Performs time stepping for the wave model core.
 * @param startTime Simulation start time.
 * @param endTime Simulation end time.
 * @param os Output stream for reporting.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-04-03
 * @date Last update : 2026-09-25
 */
void w4core_wave(const ww4_utils::DateTime &startTime,
                 const ww4_utils::DateTime &endTime, std::ostream &os);

} // namespace ww4_core
