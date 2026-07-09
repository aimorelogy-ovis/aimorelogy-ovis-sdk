#!/bin/sh

# Test Tool Launcher Script
# This script sets up the library path and runs test tools

# Get script directory
SCRIPT_SELF=$(cd "$(dirname "$0")"; pwd)

# Set library path to include the lib directory
export LD_LIBRARY_PATH=${SCRIPT_SELF}/lib:${SCRIPT_SELF}/lib/ai:${LD_LIBRARY_PATH}:/mnt/system/lib:/mnt/system/usr/lib:/mnt/system/usr/lib/3rd:/lib/3rd

# Add tools to PATH for convenience
export PATH=${SCRIPT_SELF}/tools:${PATH}

# Check if a test tool name was provided
if [ $# -eq 0 ]; then
    echo "Usage: $0 <tool_name> [args...]"
    echo ""
    echo "Available test tools:"
    if [ -d "${SCRIPT_SELF}/tools" ]; then
        ls -1 ${SCRIPT_SELF}/tools/ | grep -v "\.sh$"
    fi
    echo ""
    echo "Example:"
    echo "  $0 bmodel_test model.bmodel 10"
    echo ""
    exit 1
fi

# Get the test tool name and shift arguments
TEST_TOOL=$1
shift

# Check if test tool exists
TEST_TOOL_PATH="${SCRIPT_SELF}/tools/${TEST_TOOL}"
if [ ! -f "${TEST_TOOL_PATH}" ]; then
    echo "Error: Test tool '${TEST_TOOL}' not found in ${SCRIPT_SELF}/tools/"
    echo ""
    echo "Available test tools:"
    ls -1 ${SCRIPT_SELF}/tools/ 2>/dev/null | grep -v "\.sh$"
    exit 1
fi

if [ ! -x "${TEST_TOOL_PATH}" ]; then
    echo "Error: Test tool '${TEST_TOOL}' is not executable"
    exit 1
fi

# Run the test tool with remaining arguments
echo "Running: ${TEST_TOOL} $@"
echo "Library path: ${LD_LIBRARY_PATH}"
echo "----------------------------------------"
"${TEST_TOOL_PATH}" "$@"
