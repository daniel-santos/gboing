#!/bin/bash

((GBOING_SCRIPTS_SOURCED)) || . $(dirname "$0")/gboing.sh || exit 1

# Globals
typeset -i testSetId
typeset -i minQsortVarId
typeset -i maxQsortVarId
typeset -i minCompilerId
typeset -i maxCompilerId

export BUILD_DIR="${BUILD_DIR-/tmp/qsort-test/$$}"
typeset -i NUM_JOBS=1+$(grep 'cpu cores' /proc/cpuinfo | wc -l)
export NUM_JOBS

showUsage() {
    local argv0="$1"


    (($#)) || assert
    shift

    if (($#)); then
        echo "${BAD}ERROR${NORMAL}$: $*"
        echo
    fi

    echo "Usage: ${argv0} -h"
    echo "       ${argv0} -d <db_file> <test_set_name>"
    echo
    echo "    -d <filename>  Specify the sqlite database file to use"
    echo "                   (defaults to gboing_tests.db)"
    echo "    -h             Show this usage information"
	echo
	echo "Environment"
	echo "    BUILD_DIR      Directory to make builds in (default /tmp/qsort-test/\$\$)"
	echo "    NUM_JOBS       The number of jobs for make to run (guessed)"
    die
    exit 2
}

runQsort() {
    echo
}

main() {
    local dbFileName="gboing_tests.db"
    local testSetName

    while getopts "d:D:h" arg "$@"; do
        case "${arg}" in
            d)
                dbFileName="${OPTARG}"
                ;;
            ?|h)
                showUsage "$0"
                ;;
        esac
    done

    shift $((OPTIND-1))
    (($# == 1)) || showUsage "$0"

    testSetName="$1"

    initDb "${dbFileName}"
    typeset -i testSetId=$(getTestSetByName "${dbFileName}" "${testSetName}") ||
        die "test set not found"

    typeset -i testMask=$(
        "${SQLITE}" "${dbFileName}" "
            select type from TestSets where id=${testSetId};
        "
    ) || die

    echo "testSetId = $testSetId"
    echo "testMask  = $testMask"

    while true; do
        typeset -i testId=$(qsortGetNextTest "${dbFileName}" ${testSetId}) || die

        ((testId)) || break;

        echo testId=$testId

        env=$(qsortGetEnv "${dbFileName}" ${testId}) || die "qsortGetEnv failed"
        qsortRunTest "${dbFileName}" "${testId}" ${testMask} "${env}"
    done
}

set -o functrace
shopt -s extdebug
main "$@"
