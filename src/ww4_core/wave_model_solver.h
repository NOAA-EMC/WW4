/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file wave_model_solver.h
 * @brief Orchestration for the WW4 wave model solver.
 * @details Manages simulation state and executes model phases.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-06-24
 * @date Last update : 2026-09-25
 */

#pragma once

#include "ww4_core/solver.h"
#include <memory>
#include <vector>

namespace ww4_core {

// --- WaveModelSolver --------------------------------------------------------
/**
 * @class WaveModelSolver
 * @brief Main engine for the wave model simulation solver.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-06-24
 * @date Last update : 2026-09-25
 */
class WaveModelSolver {
public:
  /**
   * @brief Initialize the model with an assembled solver.
   * @param solver Unique pointer to the assembled solver (including physics).
   */
  void initialize(std::unique_ptr<ISolver> solver);

  /**
   * @brief Set the simulation data.
   * @param initialData Vector of initial wave data.
   */
  void setData(std::vector<double> initialData);

  /**
   * @brief Execute a single simulation step using the configured solver.
   */
  void step();

  /**
   * @brief Get a view of the current data.
   * @return Constant span of the model data.
   */
  [[nodiscard]] std::span<const double> getData() const { return data_; }

private:
  std::unique_ptr<ISolver> solver_;
  std::vector<double> data_;
};

} // namespace ww4_core
