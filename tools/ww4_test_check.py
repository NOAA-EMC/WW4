"""
      +--------------------------------------------------------+
      | WAVEWATCH IV, open source, code management by NOAA/NWS |
      +--------------------------------------------------------+

@file ww4_test_check.py
@brief A tool to check if unit tests are available for a specific file.
@details This tool searches for source files in src/ and include/ matching a
         given name, identifies user-defined routines, and checks if
         corresponding tests exist in tests/.

Usage
-----
Run the tool from the repository root or provide the file name:
    python3 tools/ww4_test_check.py --file filename

The filename should be provided without extension.

@copyright © 2026 National Weather Service, National Oceanic and Atmospheric
               Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks
               of the National Weather Service.
NWS often uses Generative AI (GenAI) for code development and refactoring. Whenever GenAI is used, NWS requires a full human review of code before it is added to its repositories.
@author Main Author(s): Aldgisl (AI Persona), Hendrik Tolman
@author Contributors: Jules (Agentic AI)
@date Initial, 2026-07-09
@date Last update : 2026-07-09
"""

import argparse
import re
from pathlib import Path
from typing import List, Set


def find_files(root_dir: Path, filename: str) -> List[Path]:
    """
    Find files matching the filename (without extension) in src and include.

    Parameters
    ----------
    root_dir : Path
        The repository root directory.
    filename : str
        The filename to search for (without extension).

    Returns
    -------
    List[Path]
        A list of matching file paths.
    """
    matches = []
    search_dirs = [root_dir / "src", root_dir / "include"]
    extensions = [".cpp", ".hpp", ".h", ".c"]

    for search_dir in search_dirs:
        if not search_dir.exists():
            continue
        for ext in extensions:
            for path in search_dir.rglob(f"{filename}{ext}"):
                matches.append(path)
    return matches


def extract_routines(file_path: Path) -> Set[str]:
    """
    Extract routine names (functions/methods) from a C++ file.

    Parameters
    ----------
    file_path : Path
        The path to the source file.

    Returns
    -------
    Set[str]
        A set of routine names found in the file.
    """
    routines = set()
    try:
        content = file_path.read_text()
    except Exception as e:
        print(f"Error reading {file_path}: {e}")
        return routines

    # Simple regex to find C++ function/method definitions or declarations
    # Matches: Type [Class::]Name(args) [const] [noexcept] [{ or ;]
    # Group 1: Return type
    # Group 2: Optional class/namespace
    # Group 3: Function/Method name
    # Group 4: Opening brace or semicolon
    pattern = re.compile(
        r"([a-zA-Z_][a-zA-Z0-9_<>, \t*&:]*)\s+"  # Return type
        r"(?:([a-zA-Z_][a-zA-Z0-9_]*)::)?"  # Optional class/namespace
        r"([a-zA-Z_][a-zA-Z0-9_]*)"  # Function/Method name
        r"\s*\([^)]*\)\s*(?:const)?\s*(?:noexcept)?\s*(?:\[\[[^\]]+\]\]\s*)?([;{])"
    )

    is_header = file_path.suffix in {".hpp", ".h"}

    for match in pattern.finditer(content):
        ret_type = match.group(1).strip()
        namespace = match.group(2)
        name = match.group(3)
        end_char = match.group(4)

        # Filter out common C++ keywords that might be misidentified as return type
        if ret_type in {"if", "for", "while", "switch", "return", "catch", "case"}:
            continue

        # Filter out matches in the std namespace (e.g., std::format)
        if namespace == "std":
            continue

        # Filter out common C++ keywords identified as routine name
        if name in {
            "if",
            "for",
            "while",
            "switch",
            "return",
            "catch",
            "format",
            "string_view",
            "span",
            "vector",
            "array",
        }:
            continue

        # In .cpp files, we only care about definitions (ending with {)
        # to avoid capturing variable declarations or function calls.
        if not is_header and end_char == ";":
            continue

        routines.add(name)

    return routines


def find_test_files(root_dir: Path, filename: str) -> List[Path]:
    """
    Find test files matching the filename in tests.

    Parameters
    ----------
    root_dir : Path
        The repository root directory.
    filename : str
        The filename to search for.

    Returns
    -------
    List[Path]
        A list of matching test file paths.
    """
    test_dir = root_dir / "tests"
    if not test_dir.exists():
        return []

    # Common naming patterns for tests
    patterns = [
        f"test_{filename}.cpp",
        f"{filename}_test.cpp",
        f"test_{filename}.hpp",
        f"L1_test_{filename}.cpp",
        f"L2_test_{filename}.cpp",
    ]
    matches = []
    for pattern in patterns:
        for path in test_dir.rglob(pattern):
            matches.append(path)
    return matches


def check_routines_in_tests(routines: Set[str], test_files: List[Path]) -> Set[str]:
    """
    Check which routines are mentioned in the test files.

    Parameters
    ----------
    routines : Set[str]
        A set of routine names to check.
    test_files : List[Path]
        A list of test file paths to search in.

    Returns
    -------
    Set[str]
        A set of routines found in the tests.
    """
    covered = set()
    if not test_files:
        return covered

    test_contents = ""
    for test_file in test_files:
        try:
            test_contents += test_file.read_text() + "\n"
        except Exception:
            pass

    for routine in routines:
        # Check if routine name is used as a word in the test content
        if re.search(r"\b" + re.escape(routine) + r"\b", test_contents):
            covered.add(routine)

    return covered


def main() -> None:
    """
    Main entry point for the test check tool.
    """
    parser = argparse.ArgumentParser(description="WAVEWATCH IV Test Check Tool")
    parser.add_argument(
        "--file",
        type=str,
        required=True,
        help="The filename to check (without extension, e.g., 'time_management')",
    )

    args = parser.parse_args()
    filename = args.file

    # Repository root is the parent of the directory containing this script
    root_dir = Path(__file__).parent.parent.resolve()

    source_files = find_files(root_dir, filename)
    if not source_files:
        print(f"No source files found for '{filename}' in src/ or include/.")
        return

    print(f"Checking unit tests for file: {filename}")
    print("-" * 40)
    print("Source files found:")
    for f in source_files:
        print(f"  - {f.relative_to(root_dir)}")

    routines = set()
    for f in source_files:
        routines.update(extract_routines(f))

    if not routines:
        print("\nNo user-defined routines identified in source files.")
    else:
        print(f"\nIdentified {len(routines)} routine(s) in source files.")

    test_files = find_test_files(root_dir, filename)
    if not test_files:
        print("\nNo test files found in tests/ matching the name.")
    else:
        print("\nTest files found:")
        for f in test_files:
            print(f"  - {f.relative_to(root_dir)}")

    covered = check_routines_in_tests(routines, test_files)

    if routines:
        print("\nRoutine Coverage:")
        print(f"{'Routine Name':<30} {'Status':<10}")
        print("-" * 40)
        for routine in sorted(list(routines)):
            status = "[OK]" if routine in covered else "[MISSING]"
            print(f"{routine:<30} {status}")

    print("-" * 40)
    if routines and len(covered) == len(routines):
        print("All identified routines have corresponding tests.")
    elif routines:
        print(f"Summary: {len(covered)}/{len(routines)} routines have tests.")
    else:
        print("Summary: No routines found to check.")


if __name__ == "__main__":
    main()
