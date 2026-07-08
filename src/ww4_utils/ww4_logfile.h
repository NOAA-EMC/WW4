/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file ww4_logfile.h
 * @brief Routines for log file output.
 * @details This header defines routines for managing log file output,
 *          duplicating the formats from WAVEWATCH III, updated for
 *          WAVEWATCH IV.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-04-01
 * @date Last update : 2026-07-07
 * @note Converted from WAVEWATCH III (ww3_shel.F90 and ww3_multi.F90).
 *       Original author: Hendrik L. Tolman.
 */

#pragma once

#include "ww4_utils/memory_utils.h"
#include "ww4_utils/time_management.h"
#include <iostream>
#include <optional>
#include <string>

namespace ww4_utils {

namespace ww4_logfile {

/**
 * @struct LogTableData
 * @brief Data structure for tabular log output tracking.
 * @var LogTableData::wlUpdated
 * @brief Water level update flag.
 * @var LogTableData::cuUpdated
 * @brief Currents update flag.
 * @var LogTableData::wiUpdated
 * @brief Winds update flag.
 * @var LogTableData::icUpdated
 * @brief Ice concentrations update flag.
 * @var LogTableData::bdUpdated
 * @brief Bottom depth update flag.
 * @var LogTableData::fieldsPerformed
 * @brief Gridded fields output flag.
 * @var LogTableData::pointsPerformed
 * @brief Point output flag.
 * @var LogTableData::restartPerformed
 * @brief Restart file output flag.
 * @var LogTableData::apiPerformed
 * @brief API output flag.
 */
struct LogTableData {
  bool wlUpdated = false;
  bool cuUpdated = false;
  bool wiUpdated = false;
  bool icUpdated = false;
  bool bdUpdated = false;
  bool fieldsPerformed = false;
  bool pointsPerformed = false;
  bool restartPerformed = false;
  bool apiPerformed = false;

  bool anyAction() const;

  void reset();
};

void writeInitialOutput(std::ostream &os, std::string_view programName);

void writeFinalOutput(std::ostream &os, std::string_view programName,
                      std::optional<double> initTime = std::nullopt,
                      std::optional<double> elapsedTotal = std::nullopt);

void writeUpdatingField(std::ostream &os, std::string_view fieldName);

void writeInterpolationInfo(std::ostream &os, const DateTime &time1,
                            const DateTime &time2);

void writeLogTableHeader(std::ostream &os);

void writeLogTableLine(std::ostream &os, const DateTime &time,
                       const LogTableData &data);

void writeLogTableFooter(std::ostream &os);

} // namespace ww4_logfile
} // namespace ww4_utils
