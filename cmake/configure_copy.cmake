# This script runs at build time via 'cmake -P'
# It uses the IN_FILE and OUT_FILE variables passed via -D

configure_file(
    "${IN_FILE}"
    "${OUT_FILE}"
    COPYONLY
)