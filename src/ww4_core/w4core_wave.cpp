/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file w4core_wave.cpp
 * @brief Implementation of the WAVEWATCH IV time stepping routine.
 * @details This file implements the w4core_wave routine.
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

#include "ww4_core/w4core_wave.h"
#include "ww4_core/w4core_init.h"
#include "ww4_utils/time_management.h"
#include "ww4_utils/ww4_input_utils.h"
#include "ww4_utils/ww4_std_out.h"
#include <algorithm>
#include <chrono>
#include <exception>
#include <iostream>
#include <thread>

namespace ww4_core {

// --- w4core_wave_uq ---------------------------------------------------------
/**
 * @brief Solver routine for Ultimate Quickest (UQ) scheme on a regular grid.
 * @param timeStep Time step in seconds.
 * @param os Output stream.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-04-03
 * @date Last update : 2026-09-25
 */
void w4core_wave_uq(double timeStep, std::ostream &os) {
  (void)timeStep;
  if (getRunConfig().produceStdOut &&
      getRunConfig().screenOutputLevel == ww4_utils::ScreenOutputLevel::Full) {
    os << "    Executing UQ regular grid solver" << std::endl;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

// --- w4core_wave_triangular -------------------------------------------------
/**
 * @brief Solver routine for triangular unstructured grid approach.
 * @param timeStep Time step in seconds.
 * @param os Output stream.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-04-03
 * @date Last update : 2026-09-25
 */
void w4core_wave_triangular(double timeStep, std::ostream &os) {
  (void)timeStep;
  if (getRunConfig().produceStdOut &&
      getRunConfig().screenOutputLevel == ww4_utils::ScreenOutputLevel::Full) {
    os << "    Executing Triangular unstructured grid solver" << std::endl;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

// --- w4core_wave_smc --------------------------------------------------------
/**
 * @brief Solver routine for Spherical Multiple-Cell (SMC) grid solver.
 * @param timeStep Time step in seconds.
 * @param os Output stream.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-04-03
 * @date Last update : 2026-09-25
 */
void w4core_wave_smc(double timeStep, std::ostream &os) {
  (void)timeStep;
  if (getRunConfig().produceStdOut &&
      getRunConfig().screenOutputLevel == ww4_utils::ScreenOutputLevel::Full) {
    os << "    Executing SMC grid solver" << std::endl;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

// --- w4core_wave ------------------------------------------------------------
/**
 * @brief Time stepping routine for the WAVEWATCH IV core.
 * @details Performs a sequence of time steps for the wave model core.
 *          Follows the architectural design of time stepping in ww3_multi.F90
 *          from WAVEWATCH III.
 * @param startTime Simulation start time.
 * @param endTime Simulation end time.
 * @param os Output stream for reporting.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-04-03
 * @date Last update : 2026-09-25
 */
void w4core_wave(const ww4_utils::DateTime &startTime,
                 const ww4_utils::DateTime &endTime, std::ostream &os) {
  try {
    // === General initialization =============================================
    if (getRunConfig().produceStdOut) {
      os << "  Time stepping (w4core_wave) from: "
         << ww4_utils::TimeManagement::toFormattedString(startTime)
         << " to: " << ww4_utils::TimeManagement::toFormattedString(endTime)
         << "\n"
         << std::endl;
    }

    // === Check consistency of starting and ending times =====================
    if (ww4_utils::TimeManagement::differenceInSeconds(startTime, endTime) <
        0.0) {
      // Explanatory comment preceding __FILE__ and __LINE__
      // Exit program when end time precedes start time
      ww4_utils::ww4_std_out::extcde(1, os, "End time before start time.",
                                     __FILE__, __LINE__);
    }

    if (!getWaveTimeData().modelTime.has_value()) {
      // Explanatory comment preceding __FILE__ and __LINE__
      // Exit program when model time is uninitialized
      ww4_utils::ww4_std_out::extcde(1, os, "Model time not initialized.",
                                     __FILE__, __LINE__);
    }

    if (*getWaveTimeData().modelTime != startTime) {
      // Explanatory comment preceding __FILE__ and __LINE__
      // Exit program when start time mismatch occurs
      ww4_utils::ww4_std_out::extcde(
          1, os, "Start time does not match model time.", __FILE__, __LINE__);
    }

    // === Assess output configurations =======================================
    ww4_utils::assessOutputConfig(startTime, endTime, getMutableRunConfig());

    // === Time stepping loop =================================================
    ww4_utils::InputUpdateState inputState;
    int consecutiveZeroSteps = 0;

    while (ww4_utils::TimeManagement::differenceInSeconds(
               *getWaveTimeData().modelTime, endTime) > 0.001) {
      // === Determine time step ==============================================
      bool headerPrinted = false;
      if (getRunConfig().produceStdOut &&
          getRunConfig().screenOutputLevel ==
              ww4_utils::ScreenOutputLevel::Full) {
        os << "  Computation step starting "
           << ww4_utils::TimeManagement::toFormattedString(
                  *getWaveTimeData().modelTime)
           << std::endl;
        headerPrinted = true;
      }

      ww4_utils::ww4_logfile::LogTableData logData;
      ww4_utils::waveTimeData waveTime = getWaveTimeData();
      ww4_utils::updateAllInputs(*waveTime.modelTime, endTime, waveTime,
                                 inputState, getRunConfig(), headerPrinted, os,
                                 logData);

      updateWaveInputTime(ww4_utils::InputType::WaterLevels,
                          waveTime.waterLevels);
      updateWaveInputTime(ww4_utils::InputType::Currents, waveTime.currents);
      updateWaveInputTime(ww4_utils::InputType::Winds, waveTime.winds);
      updateWaveInputTime(ww4_utils::InputType::IceConcentrations,
                          waveTime.iceConcentrations);
      updateWaveInputTime(ww4_utils::InputType::BottomDepth,
                          waveTime.bottomDepth);

      double inputTimeStep =
          ww4_utils::computeInputTimeStep(*getWaveTimeData().modelTime, endTime,
                                          getWaveTimeData(), getRunConfig());

      double outputTimeStep = ww4_utils::computeOutputTimeStep(
          *getWaveTimeData().modelTime, endTime, getRunConfig());

      double actualTimeStep =
          std::min({inputTimeStep, outputTimeStep, getRunConfig().timeStep});

      if (actualTimeStep < 0.0) {
        // Explanatory comment preceding __FILE__ and __LINE__
        // Exit program when negative time step occurs
        ww4_utils::ww4_std_out::extcde(1, os, "Negative time step detected.",
                                       __FILE__, __LINE__);
      } else if (actualTimeStep > 0.0 && actualTimeStep < 0.001) {
        actualTimeStep = 0.001;
      }

      if (actualTimeStep < 0.001) {
        consecutiveZeroSteps++;
      } else {
        consecutiveZeroSteps = 0;
      }

      if (consecutiveZeroSteps >= 2) {
        // Explanatory comment preceding __FILE__ and __LINE__
        // Exit program when consecutive zero steps occur
        ww4_utils::ww4_std_out::extcde(1, os,
                                       "Two consecutive zero time steps "
                                       "detected.",
                                       __FILE__, __LINE__);
      }

      if (actualTimeStep > 0.0) {
        // === Propagate solution ============================================
        switch (getRunConfig().solver) {
        case ww4_utils::SolverType::UQ:
          w4core_wave_uq(actualTimeStep, os);
          break;
        case ww4_utils::SolverType::Triangular:
          w4core_wave_triangular(actualTimeStep, os);
          break;
        case ww4_utils::SolverType::SMC:
          w4core_wave_smc(actualTimeStep, os);
          break;
        default:
          // Explanatory comment preceding __FILE__ and __LINE__
          // Exit program when unknown solver is selected
          ww4_utils::ww4_std_out::extcde(
              1, os, "No numerical solver specified or unknown solver.",
              __FILE__, __LINE__);
          break;
        }

        ww4_utils::DateTime nextTime = *getWaveTimeData().modelTime;
        ww4_utils::TimeManagement::incrementDateTime(nextTime, actualTimeStep);
        updateWaveModelTime(nextTime);
      }

      // === Perform output ===================================================
      auto printOutput = [&](const std::string_view msg, bool &logFlag) {
        if (getRunConfig().produceStdOut &&
            getRunConfig().screenOutputLevel !=
                ww4_utils::ScreenOutputLevel::None) {
          if (getRunConfig().screenOutputLevel ==
                  ww4_utils::ScreenOutputLevel::Summary &&
              !headerPrinted) {
            os << "  Computation step starting "
               << ww4_utils::TimeManagement::toFormattedString(
                      *getWaveTimeData().modelTime)
               << std::endl;
            headerPrinted = true;
          }
          if (headerPrinted) {
            os << "    " << msg << std::endl;
          }
        }
        if (getRunConfig().produceLogFile && getLogFileStream().is_open()) {
          logFlag = true;
        }
      };

      if (getRunConfig().outputFields.requested &&
          getRunConfig().outputFields.actualTime ==
              getWaveTimeData().modelTime) {
        printOutput("Performing fields output", logData.fieldsPerformed);
      }

      if (getRunConfig().outputPoints.requested &&
          getRunConfig().outputPoints.actualTime ==
              getWaveTimeData().modelTime) {
        printOutput("Performing points output", logData.pointsPerformed);
      }

      if (getRunConfig().outputRestart.requested &&
          getRunConfig().outputRestart.actualTime ==
              getWaveTimeData().modelTime) {
        printOutput("Performing restart output", logData.restartPerformed);
      }

      if (getRunConfig().outputApi.requested &&
          getRunConfig().outputApi.actualTime == getWaveTimeData().modelTime) {
        printOutput("Performing API output", logData.apiPerformed);
      }

      if (getRunConfig().produceLogFile && getLogFileStream().is_open() &&
          logData.anyAction()) {
        ww4_utils::ww4_logfile::writeLogTableLine(
            getLogFileStream(), *getWaveTimeData().modelTime, logData);
      }

      ww4_utils::updateOutputActualTimes(*getWaveTimeData().modelTime, endTime,
                                         getMutableRunConfig());
    }

  } catch (const std::exception &e) {
    // Explanatory comment preceding __FILE__ and __LINE__
    // Terminate execution on standard exception
    ww4_utils::ww4_std_out::extcde(1, os, e.what(), __FILE__, __LINE__);
  } catch (...) {
    // Explanatory comment preceding __FILE__ and __LINE__
    // Terminate execution on unknown exception
    ww4_utils::ww4_std_out::extcde(1, os, "Unknown exception in w4core_wave",
                                   __FILE__, __LINE__);
  }
}

} // namespace ww4_core
