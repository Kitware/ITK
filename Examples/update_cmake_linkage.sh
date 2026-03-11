#!/bin/bash

# Script to update CMakeLists.txt files with modern ITK module linkage
# Usage: ./update_cmake_linkage.sh <path_to_CMakeLists.txt>

set -e

if [ $# -ne 1 ]; then
    echo "Usage: $0 <path_to_CMakeLists.txt>"
    exit 1
fi

CMAKE_FILE="$1"
ITK_ROOT="/home/blowekamp/src/ITK"
WHATMODULES_SCRIPT="${ITK_ROOT}/Utilities/Maintenance/WhatModulesITK.py"

if [ ! -f "$CMAKE_FILE" ]; then
    echo "Error: CMakeLists.txt not found: $CMAKE_FILE"
    exit 1
fi

if [ ! -f "$WHATMODULES_SCRIPT" ]; then
    echo "Error: WhatModulesITK.py not found: $WHATMODULES_SCRIPT"
    exit 1
fi

# Get the directory containing the CMakeLists.txt
CMAKE_DIR=$(dirname "$CMAKE_FILE")

# Function to extract target name and source file from add_executable line
# Returns: target_name source_file
extract_target_info() {
    local line="$1"
    # Match patterns like: add_executable(TargetName SourceFile.cxx)
    if echo "$line" | grep -q "add_executable"; then
        # Extract target and source - handle multiline cases
        target=$(echo "$line" | sed -n 's/.*add_executable(\s*\([^ )]*\).*/\1/p')
        source=$(echo "$line" | sed -n 's/.*add_executable([^)]* \+\([^ )]*\.cxx\).*/\1/p')
        if [ -n "$target" ] && [ -n "$source" ]; then
            echo "$target $source"
        fi
    fi
}

# Function to get ITK modules for a source file
# Returns: space-separated list of modules
get_itk_modules() {
    local source_file="$1"
    local full_path="${CMAKE_DIR}/${source_file}"

    if [ ! -f "$full_path" ]; then
        echo "Warning: Source file not found: $full_path" >&2
        return 1
    fi

    # Run WhatModulesITK.py and extract module names
    local modules=$(python "$WHATMODULES_SCRIPT" --link "$ITK_ROOT" "$full_path" 2>/dev/null | \
        grep -A 100 "target_link_libraries" | \
        grep "ITK" | \
        grep -v "YourTarget" | \
        grep -v "PRIVATE" | \
        sed 's/^[[:space:]]*//' | \
        sed 's/[[:space:]]*$//' | \
        tr '\n' ' ')

    echo "$modules"
}

# Function to replace target_link_libraries line
replace_linkage() {
    local target="$1"
    local modules="$2"
    local temp_file="${CMAKE_FILE}.tmp"

    # Build the new target_link_libraries line (trim trailing spaces)
    modules=$(echo "$modules" | sed 's/[[:space:]]*$//')
    local new_line="target_link_libraries($target PRIVATE $modules)"

    # Use sed to replace the target_link_libraries line
    # Match the exact target name after target_link_libraries(
    sed -E "s|^target_link_libraries\($target [^\)]*\)|$new_line|g" "$CMAKE_FILE" > "$temp_file"

    mv "$temp_file" "$CMAKE_FILE"
}

# Main processing
echo "Processing $CMAKE_FILE..."
echo ""

# Create a temporary file to store target-source mappings
temp_targets=$(mktemp)

# Parse the CMakeLists.txt to find all add_executable commands
# Handle both single-line and multi-line add_executable statements
grep -n "add_executable" "$CMAKE_FILE" | while IFS=: read -r line_num line_content; do
    # Look ahead a few lines to capture multi-line add_executable
    next_lines=$(sed -n "${line_num},$((line_num+10))p" "$CMAKE_FILE")

    # Extract target name - could be on same line or next line
    target=""
    if echo "$line_content" | grep -q "add_executable([^)]*)"; then
        # Single line case
        target=$(echo "$line_content" | sed -n 's/.*add_executable(\s*\([^ )]*\).*/\1/p')
    else
        # Multi-line case - target is likely on the next line
        target=$(echo "$next_lines" | head -5 | grep -v "add_executable" | grep -v "^[[:space:]]*)" | grep -oP '^[[:space:]]*\K[^[:space:]]+' | head -1)
    fi

    # Look for the source file (.cxx)
    source=$(echo "$next_lines" | grep -oP '[^\s()]+\.cxx' | head -1)

    if [ -n "$target" ] && [ -n "$source" ]; then
        echo "$target|$source" >> "$temp_targets"
    fi
done

# Process each target
while IFS='|' read -r target source; do
    echo "Processing target: $target (source: $source)"

    # Get the ITK modules
    modules=$(get_itk_modules "$source")

    if [ -z "$modules" ]; then
        echo "  Warning: No modules found for $source"
        continue
    fi

    echo "  Modules: $modules"

    # Replace the linkage
    replace_linkage "$target" "$modules"

    echo "  Updated linkage for $target"
    echo ""
done < "$temp_targets"

# Cleanup
rm -f "$temp_targets"

echo "Done! Updated $CMAKE_FILE"
echo "Please review the changes before committing."
