#!/bin/bash
#      +--------------------------------------------------------+
#      | WAVEWATCH IV, open source, code management by NOAA/NWS |
#      +--------------------------------------------------------+
#
# @file ww4_test_check.sh
# @brief Shell wrapper for the WAVEWATCH IV (WW4) test check tool.
# @details This script identifies the repository root and executes the
#          underlying Python unit test check tool.
#
# Usage
# -----
# Run the unit test check tool from any directory within the clone:
#    ww4_test_check.sh --file <filename>
#
# Options:
#   --file FILE       The filename to check (without extension).
#
# @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
#               Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks
#               of the National Weather Service.
# NWS often uses Generative AI (GenAI) for code development and refactoring. Whenever GenAI is used, NWS requires a full human review of code before it is added to its repositories.
# @author Main Author(s): Aldgisl (AI Persona), Hendrik Tolman
# @author Contributors: Jules (Agentic AI)
# @date Initial, 2026-03-30
# @date Last Update, 2026-11-04

# Identify the repository root (parent of the directory containing this script)
TOOLS_DIR=$(cd "$(dirname "$0")" && pwd)
ACTIVE_CLONE=$(cd "$TOOLS_DIR/.." && pwd)

CHECK_TOOL="$ACTIVE_CLONE/tools/ww4_test_check.py"

if [ ! -f "$CHECK_TOOL" ]; then
    echo "Error: Test check tool not found at $CHECK_TOOL"
    exit 1
fi

python3 "$CHECK_TOOL" "$@"
