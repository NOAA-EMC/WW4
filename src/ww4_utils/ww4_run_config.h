/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file ww4_run_config.h
 * @brief Service routine for processing run-time configuration YAML file.
 * @details This header defines the RunConfig structure and the
 *          loadRunConfig function, which reads and processes the
 *          run-time configuration for WAVEWATCH IV.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI), Kit Stokes, Jessica Meixner
 * @date Initial, 2026-04-03
 * @date Last update : 2026-09-28
 */

#pragma once

#include "ww4_utils/time_management.h"
#include "ww4_utils/ww4_output_utils.h"
#include <iostream>
#include <optional>
#include <string_view>
#include <vector>

namespace ww4_utils {

// --- InputFieldOption -------------------------------------------------------
/**
 * @enum SolverType
 * @brief Numerical solver options for WAVEWATCH IV.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @var SolverType::Undefined
 * @brief Mandatory solver selection not specified.
 * @var SolverType::UQ
 * @brief Regular grid solver using Ultimate Quickest scheme.
 * @var SolverType::Triangular
 * @brief Triangular unstructured grid solver.
 * @var SolverType::SMC
 * @brief Spherical Multiple-Cell (SMC) grid solver.
 */
enum class SolverType { Undefined, UQ, Triangular, SMC };

/**
 * @enum InputDissipationScheme
 * @brief Options for input and dissipation physical source terms in WAVEWATCH
 * IV.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @var InputDissipationScheme::Undefined
 * @brief Mandatory selection not specified.
 * @var InputDissipationScheme::DoNotUse
 * @brief Do not use input and dissipation source terms.
 * @var InputDissipationScheme::ST1
 * @brief ST1 input and dissipation scheme.
 * @var InputDissipationScheme::ST2
 * @brief ST2 input and dissipation scheme.
 * @var InputDissipationScheme::ST4
 * @brief ST4 input and dissipation scheme.
 * @var InputDissipationScheme::ST6
 * @brief ST6 input and dissipation scheme.
 */
enum class InputDissipationScheme { Undefined, DoNotUse, ST1, ST2, ST4, ST6 };

/**
 * @enum NonlinearScheme
 * @brief Options for nonlinear interaction physical source terms in WAVEWATCH
 * IV.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @var NonlinearScheme::Undefined
 * @brief Mandatory selection not specified.
 * @var NonlinearScheme::DoNotUse
 * @brief Do not use nonlinear interaction source terms.
 * @var NonlinearScheme::NL1
 * @brief NL1 nonlinear interaction scheme.
 * @var NonlinearScheme::NL2
 * @brief NL2 nonlinear interaction scheme.
 * @var NonlinearScheme::NL3
 * @brief NL3 nonlinear interaction scheme.
 */
enum class NonlinearScheme { Undefined, DoNotUse, NL1, NL2, NL3 };

/**
 * @enum LinearInputScheme
 * @brief Options for linear wave growth physical source terms in WAVEWATCH IV.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @var LinearInputScheme::Undefined
 * @brief Mandatory selection not specified.
 * @var LinearInputScheme::DoNotUse
 * @brief Do not use linear input source terms.
 * @var LinearInputScheme::LN1
 * @brief LN1 linear input scheme.
 */
enum class LinearInputScheme { Undefined, DoNotUse, LN1 };

/**
 * @enum BottomFrictionScheme
 * @brief Options for bottom friction physical source terms in WAVEWATCH IV.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @var BottomFrictionScheme::Undefined
 * @brief Mandatory selection not specified.
 * @var BottomFrictionScheme::DoNotUse
 * @brief Do not use bottom friction source terms.
 * @var BottomFrictionScheme::BT1
 * @brief BT1 bottom friction scheme.
 * @var BottomFrictionScheme::BT4
 * @brief BT4 bottom friction scheme.
 */
enum class BottomFrictionScheme { Undefined, DoNotUse, BT1, BT4 };

/**
 * @enum InputFieldOption
 * @brief Options for model input fields.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI), Kit Stokes, Jessica Meixner
 * @date Initial, 2026-04-03
 * @date Last update : 2026-09-28
 * @var InputFieldOption::Undefined
 * @brief Mandatory field not yet defined.
 * @var InputFieldOption::None
 * @brief No input, data set to zero.
 * @var InputFieldOption::FromFile
 * @brief Input from file.
 * @var InputFieldOption::FromCoupling
 * @brief Input from coupling.
 * @var InputFieldOption::Homogeneous
 * @brief Homogeneous field.
 * @var InputFieldOption::FromGrid
 * @brief Static data read as part of the grid data.
 */
enum class InputFieldOption {
  Undefined,
  None,
  FromFile,
  FromCoupling,
  Homogeneous,
  FromGrid
};

// --- ScreenOutputLevel ------------------------------------------------------
/**
 * @enum ScreenOutputLevel
 * @brief Options for the level of output to standard output during time
 * stepping.
 * @author Main Author(s): Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI), Kit Stokes, Jessica Meixner
 * @date Initial, 2026-04-03
 * @date Last update : 2026-09-28
 * @var ScreenOutputLevel::None
 * @brief No output in the time stepping loop.
 * @var ScreenOutputLevel::Summary
 * @brief Output if input field updated or output produced.
 * @var ScreenOutputLevel::Full
 * @brief Standard full output for every step.
 */
enum class ScreenOutputLevel { None, Summary, Full };

// --- EchoOption -------------------------------------------------------------
/**
 * @enum EchoOption
 * @brief Options for echoing input data to standard output and log files.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI), Kit Stokes, Jessica Meixner
 * @date Initial, 2026-04-03
 * @date Last update : 2026-09-28
 * @var EchoOption::None
 * @brief No echoing of input data.
 * @var EchoOption::Summary
 * @brief Produce a summary of the input data.
 * @var EchoOption::Full
 * @brief Full reproduction of the user input data.
 */
enum class EchoOption { None, Summary, Full };

// --- SpectralConfig ---------------------------------------------------------
/**
 * @struct SpectralConfig
 * @brief Parameters defining the spectral space.
 * @details Defines discrete frequency and direction parameters for setting up
 * the spectral space based on WAVEWATCH III.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-04-03
 * @date Last update : 2026-09-28
 * @var SpectralConfig::numDirections
 * @brief Number of discrete directions (default: 36).
 * @var SpectralConfig::numFrequencies
 * @brief Number of discrete frequencies (default: 50).
 * @var SpectralConfig::freqIncrementFactor
 * @brief Frequency increment factor X (default: 1.07).
 * @var SpectralConfig::firstFrequency
 * @brief First discrete frequency in Hz (default: 0.035).
 * @var SpectralConfig::firstDirectionOffset
 * @brief Offset of the first direction as a fraction of the directional step
 * in range [0.0, 1.0] (default: 0.5).
 */
struct SpectralConfig {
  int numDirections = 36;
  int numFrequencies = 50;
  double freqIncrementFactor = 1.07;
  double firstFrequency = 0.035;
  double firstDirectionOffset = 0.5;
};

// --- HomogeneousDataPoint ---------------------------------------------------
/**
 * @struct HomogeneousDataPoint
 * @brief Data point for a homogeneous input field.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI), Kit Stokes, Jessica Meixner
 * @date Initial, 2026-04-03
 * @date Last update : 2026-09-28
 * @var HomogeneousDataPoint::time
 * @brief Time of the data point.
 * @var HomogeneousDataPoint::values
 * @brief Values of the data point.
 */
struct HomogeneousDataPoint {
  DateTime time;
  std::vector<double> values;
};

// --- RunConfig --------------------------------------------------------------
/**
 * @struct RunConfig
 * @brief Configuration for the run-time environment.
 * @details Stores the calendar type, output preferences, and input options.
 * @author Main Author(s): Hendrik L. Tolman, Aldgisl (AI Persona)
 * @author Contributors: Jules (Agentic AI), Kit Stokes, Jessica Meixner
 * @date Initial, 2026-04-03
 * @date Last update : 2026-09-28
 * @var RunConfig::calendarType
 * @brief Calendar type.
 * @var RunConfig::produceStdOut
 * @brief Screen output flag.
 * @var RunConfig::produceLogFile
 * @brief Log file flag.
 * @var RunConfig::dryRun
 * @brief Dry run flag.
 * @var RunConfig::propagateX
 * @brief Propagation flag in x-direction.
 * @var RunConfig::propagateY
 * @brief Propagation flag in y-direction.
 * @var RunConfig::propagateTheta
 * @brief Propagation flag in theta-direction.
 * @var RunConfig::propagateK
 * @brief Propagation flag in k-direction.
 * @var RunConfig::sourceTerms
 * @brief Source terms flag.
 * @var RunConfig::solver
 * @brief Selected numerical solver type.
 * @var RunConfig::waterLevels
 * @brief Water levels option.
 * @var RunConfig::currents
 * @brief Currents option.
 * @var RunConfig::winds
 * @brief Winds option.
 * @var RunConfig::iceConcentrations
 * @brief Ice concentrations option.
 * @var RunConfig::bottomDepth
 * @brief Bottom depth option.
 * @var RunConfig::echoHomInput
 * @brief Echo input flag.
 * @var RunConfig::screenOutputLevel
 * @brief Screen output level.
 * @var RunConfig::homogeneousWaterLevels
 * @brief Homogeneous water levels data.
 * @var RunConfig::homogeneousCurrents
 * @brief Homogeneous currents data.
 * @var RunConfig::homogeneousWinds
 * @brief Homogeneous winds data.
 * @var RunConfig::homogeneousIceConcentrations
 * @brief Homogeneous ice concentrations data.
 * @var RunConfig::homogeneousBottomDepth
 * @brief Homogeneous bottom depth data.
 * @var RunConfig::spectralSpace
 * @brief Parameters defining the spectral space.
 * @var RunConfig::timeStep
 * @brief Model time step in seconds.
 * @var RunConfig::outputApi
 * @brief API output configuration.
 * @var RunConfig::outputFields
 * @brief Gridded fields output configuration.
 * @var RunConfig::outputPoints
 * @brief Point output configuration.
 * @var RunConfig::outputRestart
 * @brief Restart file output configuration.
 */
struct RunConfig {
  TimeManagement::CalendarType calendarType =
      TimeManagement::CalendarType::Standard;
  bool produceStdOut = true;
  bool produceLogFile = true;
  bool dryRun = false;
  bool propagateX = true;
  bool propagateY = true;
  bool propagateTheta = true;
  bool propagateK = true;
  bool sourceTerms = true;

  SolverType solver = SolverType::Undefined;
  InputDissipationScheme inputDissipation = InputDissipationScheme::Undefined;
  NonlinearScheme nonlinearInteractions = NonlinearScheme::Undefined;
  LinearInputScheme linearInput = LinearInputScheme::Undefined;
  BottomFrictionScheme bottomFriction = BottomFrictionScheme::Undefined;

  InputFieldOption waterLevels = InputFieldOption::Undefined;
  InputFieldOption currents = InputFieldOption::Undefined;
  InputFieldOption winds = InputFieldOption::Undefined;
  InputFieldOption iceConcentrations = InputFieldOption::Undefined;
  InputFieldOption bottomDepth = InputFieldOption::FromGrid;

  EchoOption echoHomInput = EchoOption::Summary;

  ScreenOutputLevel screenOutputLevel = ScreenOutputLevel::Summary;

  std::vector<HomogeneousDataPoint> homogeneousWaterLevels;
  std::vector<HomogeneousDataPoint> homogeneousCurrents;
  std::vector<HomogeneousDataPoint> homogeneousWinds;
  std::vector<HomogeneousDataPoint> homogeneousIceConcentrations;
  std::vector<HomogeneousDataPoint> homogeneousBottomDepth;

  SpectralConfig spectralSpace;

  double timeStep = -1.0;

  OutputConfig outputApi;

  OutputConfig outputFields;
  OutputConfig outputPoints;
  OutputConfig outputRestart;
};

// --- cleanValue -------------------------------------------------------------
std::string_view cleanValue(std::string_view s);

// --- loadRunConfig ----------------------------------------------------------
std::optional<RunConfig> loadRunConfig(std::string_view filename,
                                       std::ostream &os) noexcept;

// --- reportRunConfig --------------------------------------------------------
void reportRunConfig(const RunConfig &config, std::ostream &os);

} // namespace ww4_utils
