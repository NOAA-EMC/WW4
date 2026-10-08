# WAVEWATCH IV Architecture

### Key Components

1.  **ISolver**: The primary integration strategy. It defines the contract for numerical solvers that combine dynamics (propagation) and physics (source terms) to advance the model state.
2.  **ISourceTerm**: The physical strategy interface for WAVEWATCH IV sub-source terms.
3.  **Source Term Implementations**:
    - **ComputeAllSources**: Unified routine called directly by each solver. Manages and executes physical `ISourceTerm` sub-source terms instantiated via `SchemeFactory`.
    - **Linear Input**: `SourceTermLN1` (LN1).
    - **Input and Dissipation**: `SourceTermST1` (ST1), `SourceTermST2` (ST2), `SourceTermST4` (ST4), and `SourceTermST6` (ST6).
    - **Nonlinear Interactions**: `SourceTermNL1` (NL1), `SourceTermNL2` (NL2), and `SourceTermNL3` (NL3).
    - **Bottom Friction**: `SourceTermBT1` (BT1) and `SourceTermBT4` (BT4).
4.  **Solvers (`SolverUQ`, `SolverTriangular`, `SolverSMC`)**: Concrete numerical solvers (Ultimate Quickest regular grid, Triangular unstructured grid, and SMC grid) that directly invoke `ComputeAllSources` during their `solve` phase.
5.  **SchemeFactory**: Responsible for instantiating individual model components (solvers and source terms).
6.  **WaveModelSolver**: The high-level orchestrator solver. It manages the simulation loop and triggers the configured solver at each time step.
This document describes the high-level architecture of WAVEWATCH IV (WW4) using a Mermaid diagram.

## Component Diagram

```mermaid
graph TD
    subgraph "Programs"
        SA[ww4_standalone]
    end

    subgraph "Core Library"
        CORE[ww4_core]
    end

    subgraph "Utility Library"
        UTILS[ww4_utils]
    end

    subgraph "Tests"
        L1L2[tests]
    end

    subgraph "External Dependencies"
        YAML[yaml-cpp]
        GTEST[googletest]
    end

    SA --> CORE
    SA --> UTILS
    CORE --> UTILS

    L1L2 --> CORE
    L1L2 --> UTILS
    L1L2 --> GTEST

    UTILS --> YAML
```

## Call Structure

The following diagram illustrates the sequence of function calls when running the `ww4_standalone` program.

```mermaid
sequenceDiagram
    participant SA as ww4_standalone
    participant INIT as w4core_init
    participant WAVE as w4core_wave
    participant FINL as w4core_finalize
    participant UTILS as ww4_utils

    SA->>UTILS: loadStandaloneConfig()
    SA->>INIT: w4core_init(startTime)
    INIT->>UTILS: loadRunConfig()
    INIT->>UTILS: setCalendarType()
    INIT->>UTILS: ww4_input_update()
    INIT->>UTILS: writeInitialOutput()
    INIT->>UTILS: writeLogTableHeader()

    SA->>WAVE: w4core_wave(startTime, endTime)
    loop until endTime
        WAVE->>UTILS: assessOutputConfig()
        WAVE->>UTILS: updateAllInputs()
        WAVE->>UTILS: computeInputTimeStep()
        WAVE->>UTILS: computeOutputTimeStep()
        WAVE->>UTILS: updateOutputActualTimes()
        Note over WAVE: Propagate Solution
    end

    SA->>FINL: w4core_finalize(endTime)
```

## Description of Components

- **Programs**: Contains end-user applications. `ww4_standalone` is a simplified environment for running the wave model core.
- **Core Library (`ww4_core`)**: Implements the main wave model routines, including initialization (`w4core_init`), time stepping (`w4core_wave`), finalization (`w4core_finalize`), numerical solvers, and physical source terms located under `src/ww4_core/source_terms/`.
- **Utility Library (`ww4_utils`)**: Provides common functionality such as time management, configuration loading, logging, and standard output utilities.
- **Tests**: Contains unit tests (Level 1, prefixed with `L1_test_`) and integration tests (Level 2, prefixed with `L2_test_`) using the Google Test framework.
- **External Dependencies**:
    - `yaml-cpp`: Used for parsing YAML configuration files.
    - `googletest`: Used for unit and integration testing.

## Integrated Runtime Solver Selection

WW4 uses an integrated **Strategy** and **Factory** design pattern. To support complex interleaving of physics and dynamics (e.g., implicit sub-stepping), physical source terms are managed and called directly by a **Solver**.

```mermaid
classDiagram
    class WaveModelSolver {
        -unique_ptr~ISolver~ solver_
        -vector~double~ data_
        +initialize(unique_ptr~ISolver~ solver)
        +setData(vector~double~ initialData)
        +step()
        +getData() span~const double~
    }

    class ISolver {
        <<interface>>
        +getName() string_view
        +addSourceTerm(unique_ptr~ISourceTerm~ source)*
        +solve(span~double~ data)*
    }

    class ISourceTerm {
        <<interface>>
        +getName() string_view
        +calculate(span~double~ data)*
    }

    class SolverUQ {
        +getName() string_view
        +addSourceTerm(unique_ptr~ISourceTerm~ source)
        +solve(span~double~ data)
    }

    class SolverTriangular {
        +getName() string_view
        +addSourceTerm(unique_ptr~ISourceTerm~ source)
        +solve(span~double~ data)
    }

    class SolverSMC {
        +getName() string_view
        +addSourceTerm(unique_ptr~ISourceTerm~ source)
        +solve(span~double~ data)
    }

    class ComputeAllSources {
        -unique_ptr~ISourceTerm~ linearInputTerm_
        -unique_ptr~ISourceTerm~ inputDissipationTerm_
        -unique_ptr~ISourceTerm~ nonlinearTerm_
        -unique_ptr~ISourceTerm~ bottomFrictionTerm_
        +getName() string_view
        +init()
        +calculate(span~double~ data)
    }

    namespace LinearInput {
        class SourceTermLN1 {
            <<Linear Input>>
            +getName() string_view
            +calculate(span~double~ data)
        }
    }

    namespace InputAndDissipation {
        class SourceTermST1 {
            <<Input and Dissipation>>
            +getName() string_view
            +calculate(span~double~ data)
        }
        class SourceTermST2 {
            <<Input and Dissipation>>
            +getName() string_view
            +calculate(span~double~ data)
        }
        class SourceTermST4 {
            <<Input and Dissipation>>
            +getName() string_view
            +calculate(span~double~ data)
        }
        class SourceTermST6 {
            <<Input and Dissipation>>
            +getName() string_view
            +calculate(span~double~ data)
        }
    }

    namespace NonlinearInteractions {
        class SourceTermNL1 {
            <<Nonlinear Interactions>>
            +getName() string_view
            +calculate(span~double~ data)
        }
        class SourceTermNL2 {
            <<Nonlinear Interactions>>
            +getName() string_view
            +calculate(span~double~ data)
        }
        class SourceTermNL3 {
            <<Nonlinear Interactions>>
            +getName() string_view
            +calculate(span~double~ data)
        }
    }

    namespace BottomFriction {
        class SourceTermBT1 {
            <<Bottom Friction>>
            +getName() string_view
            +calculate(span~double~ data)
        }
        class SourceTermBT4 {
            <<Bottom Friction>>
            +getName() string_view
            +calculate(span~double~ data)
        }
    }

    class SchemeFactory {
        +createSolver(string name) unique_ptr~ISolver~
        +createSourceTerm(string name) unique_ptr~ISourceTerm~
    }

    WaveModelSolver o-- ISolver : orchestrates
    ISolver <|-- SolverUQ
    ISolver <|-- SolverTriangular
    ISolver <|-- SolverSMC
    SolverUQ --> ComputeAllSources : calls
    SolverTriangular --> ComputeAllSources : calls
    SolverSMC --> ComputeAllSources : calls
    ISourceTerm <|-- ComputeAllSources
    ISourceTerm <|-- SourceTermLN1
    ISourceTerm <|-- SourceTermST1
    ISourceTerm <|-- SourceTermST2
    ISourceTerm <|-- SourceTermST4
    ISourceTerm <|-- SourceTermST6
    ISourceTerm <|-- SourceTermNL1
    ISourceTerm <|-- SourceTermNL2
    ISourceTerm <|-- SourceTermNL3
    ISourceTerm <|-- SourceTermBT1
    ISourceTerm <|-- SourceTermBT4
    ComputeAllSources o-- ISourceTerm : manages & calls
    SchemeFactory ..> ISolver : creates
    SchemeFactory ..> ISourceTerm : creates

    SourceTermLN1 .. SourceTermST1
    SourceTermST6 .. SourceTermNL1
    SourceTermNL3 .. SourceTermBT1
```