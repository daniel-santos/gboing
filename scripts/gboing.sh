#!/bin/bash

export GBOING_DIR="/home/daniel/proj/gboing"
export GBOING_SCRIPTS_DIR="${GBOING_DIR}/scripts"
export PATH="${GBOING_SCRIPTS_DIR}:${PATH#${GBOING_SCRIPTS_DIR}:}"

SQLITE="${SQLITE:-$(which sqlite  2>/dev/null)}"
SQLITE="${SQLITE:-$(which sqlite3 2>/dev/null)}"
SQLITE="${SQLITE:-$(which sqlite2 2>/dev/null)}"

if [[ ! -x "${SQLITE}" ]]; then
    echo "WARNING: sqlite not found" >&2
else
    export SQLITE
fi

. "${GBOING_SCRIPTS_DIR}/funcs.sh"
