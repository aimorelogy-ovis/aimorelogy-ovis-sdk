#!/bin/sh

# =============================================================================
# sample_lsc run script
# =============================================================================
# Usage: ./run.sh [online|offline|offline_dir]
# Default: offline_dir
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BINARY="${SCRIPT_DIR}/sample_lsc_cali"
MODE="${1:-offline_dir}"
LOG_FILE="${SCRIPT_DIR}/log.txt"

# Check binary exists
if [ ! -f "${BINARY}" ]; then
    echo "Error: sample_lsc_cali binary not found"
    echo "Run 'make clean && make -j' in sample_lsc directory first"
    exit 1
fi

# =============================================================================
# Peak memory monitoring: use /usr/bin/time -v to get max resident set size
# =============================================================================
run_with_monitor() {
    _time_log="${LOG_FILE}.time"
    echo "--------------------------------------------"
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] Starting..." >> "$LOG_FILE"
    /usr/bin/time -v "$@" 2>"$_time_log"
    _exit_code=$?
    # Extract peak memory (Maximum resident set size)
    _peak_kb=$(grep 'Maximum resident set size' "$_time_log" 2>/dev/null | awk '{print $NF}')
    if [ -n "$_peak_kb" ]; then
        _peak_mb=$(echo "scale=2; ${_peak_kb} / 1024" | bc 2>/dev/null || echo "N/A")
        echo "[$(date '+%Y-%m-%d %H:%M:%S')] Peak memory (MaxRSS): ${_peak_kb} kB (${_peak_mb} MB)" >> "$LOG_FILE"
        echo "Peak memory: ${_peak_kb} kB (${_peak_mb} MB)"
    fi
    rm -f "$_time_log"
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] Finished (exit code: ${_exit_code})" >> "$LOG_FILE"
    echo ""
    return $_exit_code
}

# =============================================================================
# Mode selection
# =============================================================================

case "${MODE}" in

# ----------------------------------------------------------------------------
# online mode - capture Raw frames from VI pipeline for calibration
# Requires camera sensor and /mnt/data/sensor_cfg.ini on device
# ----------------------------------------------------------------------------
online)
    echo "============================================"
    echo "  Mode: online"
    echo "============================================"

    # Config
    COLOR_TEMP=5000      # Color temperature (K)
    VERIFY=0             # 1=run verify, 0=skip

    # Command line overrides: ./run.sh online [color_temp] [verify]
    COLOR_TEMP="${2:-$COLOR_TEMP}"
    VERIFY="${3:-$VERIFY}"

    echo "Color temp: ${COLOR_TEMP}K"
    echo "Verify: ${VERIFY}"
    echo "Press Ctrl+C or Ctrl+D to exit"

    run_with_monitor ${BINARY} online ${COLOR_TEMP} ${VERIFY}
    ;;

# ----------------------------------------------------------------------------
# offline mode - single file calibration
# ----------------------------------------------------------------------------
offline)
    echo "============================================"
    echo "  Mode: offline (single file)"
    echo "============================================"

    # Config
    RAW_FILE="/mnt/sd/sample_lsc/frame.raw"
    WIDTH=2560
    HEIGHT=1920
    BAYER_ID=3           # 0=RG, 1=GR, 2=GB, 3=BG
    COLOR_TEMP=2800      # Color temperature (K)
    CALIB_MODE=1         # 0=MESH_CHROMA_MODE(2-pass), 1=MESH_CHROMA_LUMA_MODE(single)
    VERIFY=0             # 1=run verify, 0=skip

    # Command line overrides: ./run.sh offline <raw> <w> <h> <bayer> <color_temp> <calib_mode> <verify>
    RAW_FILE="${2:-$RAW_FILE}"
    WIDTH="${3:-$WIDTH}"
    HEIGHT="${4:-$HEIGHT}"
    BAYER_ID="${5:-$BAYER_ID}"
    COLOR_TEMP="${6:-$COLOR_TEMP}"
    CALIB_MODE="${7:-$CALIB_MODE}"
    VERIFY="${8:-$VERIFY}"

    echo "Raw file: ${RAW_FILE}"
    echo "Resolution: ${WIDTH}x${HEIGHT}"
    echo "Bayer: ${BAYER_ID}"
    echo "Color temp: ${COLOR_TEMP}K"
    if [ "${CALIB_MODE}" -eq 0 ]; then
        MODE_STR="2-pass"
    else
        MODE_STR="single"
    fi
    echo "Calib mode: ${CALIB_MODE} (${MODE_STR})"
    echo "Verify: ${VERIFY}"

    if [ ! -f "${RAW_FILE}" ]; then
        echo "Error: Raw file not found: ${RAW_FILE}"
        exit 1
    fi

    run_with_monitor ${BINARY} offline ${RAW_FILE} ${WIDTH} ${HEIGHT} ${BAYER_ID} ${COLOR_TEMP} ${CALIB_MODE} ${VERIFY}
    ;;

# ----------------------------------------------------------------------------
# offline_dir mode - directory traversal batch calibration (default)
# Directory structure: res/A(2800K)/*.raw + *.txt
#                      res/D75(7500K)/*.raw + *.txt
# ----------------------------------------------------------------------------
offline_dir)
    echo "============================================"
    echo "  Mode: offline_dir (directory traversal)"
    echo "============================================"

    # Config
    BASE_DIR="${SCRIPT_DIR}/res"    # Raw file root directory
    CALIB_MODE=1                     # 0=MESH_CHROMA_MODE(2-pass), 1=MESH_CHROMA_LUMA_MODE(single)
    VERIFY=0                         # 1=run verify, 0=skip

    # Command line overrides: ./run.sh offline_dir <calib_mode> <verify>
    CALIB_MODE="${2:-$CALIB_MODE}"
    VERIFY="${3:-$VERIFY}"

    echo "Data directory: ${BASE_DIR}"
    if [ "${CALIB_MODE}" -eq 0 ]; then
        MODE_STR="2-pass"
    else
        MODE_STR="single"
    fi
    echo "Calib mode: ${CALIB_MODE} (${MODE_STR})"
    echo "Verify: ${VERIFY}"

    if [ ! -d "${BASE_DIR}" ]; then
        echo "Error: Data directory not found: ${BASE_DIR}"
        exit 1
    fi

    run_with_monitor ${BINARY} offline_dir ${BASE_DIR} ${CALIB_MODE} ${VERIFY}
    ;;

*)
    echo "Usage: $0 [online|offline|offline_dir]"
    echo ""
    echo "  online       Online mode (capture from VI in real-time)"
    echo "  offline      Offline mode (single file)"
    echo "  offline_dir  Offline directory mode (default)"
    exit 1
    ;;

esac

echo "Done"
