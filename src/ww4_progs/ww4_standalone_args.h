/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file ww4_standalone_args.h
 * @brief Command line argument parsing for the stand-alone program.
 * @details This header defines the StandaloneArgs structure and functions for
 *          parsing and reporting command line start and end simulation times.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-10-22
 * @date Last update : 2026-10-22
 */

#pragma once

#include "ww4_utils/time_management.h"
#include <iostream>
#include <optional>

namespace ww4_progs {

// --- StandaloneArgs -------------------------------------------------------
/**
 * @struct StandaloneArgs
 * @brief Command line arguments for the ww4_standalone program.
 * @details Stores the start and end times for the simulation.
 * @author Main Author(s): Hendrik L. Tolman, Aldgisl (AI Persona)
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-10-22
 * @date Last update : 2026-10-22
 * @var StandaloneArgs::startTime
 * @brief Simulation start time.
 * @var StandaloneArgs::endTime
 * @brief Simulation end time.
 */
struct StandaloneArgs {
  ww4_utils::DateTime startTime;
  ww4_utils::DateTime endTime;
};

// --- parseStandaloneArgs ---------------------------------------------------
/**
 * @brief Parses simulation start and end times from command line arguments.
 * @param argc Command line argument count.
 * @param argv Command line argument values.
 * @param os Output stream for reporting error/usage messages.
 * @return A StandaloneArgs structure if successful, or std::nullopt.
 */
std::optional<StandaloneArgs> parseStandaloneArgs(int argc, char **argv,
                                                  std::ostream &os);

// --- reportStandaloneArgs -------------------------------------------------
/**
 * @brief Reports the stand-alone configuration to the provided output stream.
 * @param args The StandaloneArgs structure to report.
 * @param os The output stream to write to.
 */
void reportStandaloneArgs(const StandaloneArgs &args, std::ostream &os);

} // namespace ww4_progs
