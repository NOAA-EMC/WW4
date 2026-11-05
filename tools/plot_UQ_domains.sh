#!/bin/bash
#      +--------------------------------------------------------+
#      | WAVEWATCH IV, open source, code management by NOAA/NWS |
#      +--------------------------------------------------------+
#
# @file plot_UQ_domains.sh
# @brief Shell wrapper for the WAVEWATCH IV UQ domain partitioning plotting tool.
# @details This script identifies the repository root and executes the
#          underlying Python plotting tool (plot_UQ_domains.py).
#
# Usage
# -----
# Run the tool from any directory within the clone:
#    plot_UQ_domains.sh --input <partition_file.nc> [options]
#
# @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
#               Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks
#               of the National Weather Service.
# NWS often uses Generative AI (GenAI) for code development and refactoring. Whenever GenAI is used, NWS requires a full human review of code before it is added to its repositories.
# @author Main Author(s): Aldgisl (AI Persona), Hendrik Tolman
# @author Contributors: Jules (Agentic AI)
# @date Initial, 2026-11-04
# @date Last update : 2026-11-05

# Identify the repository root (parent of the directory containing this script)
TOOLS_DIR=$(cd "$(dirname "$0")" && pwd)
ACTIVE_CLONE=$(cd "$TOOLS_DIR/.." && pwd)

PLOT_TOOL="$ACTIVE_CLONE/tools/plot_UQ_domains.py"

if [ ! -f "$PLOT_TOOL" ]; then
    echo "Error: Plotting tool not found at $PLOT_TOOL"
    exit 1
fi

python3 "$PLOT_TOOL" "$@"
