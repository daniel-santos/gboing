#!/bin/bash

((GBOING_SCRIPTS_SOURCED)) || . $(dirname "$0")/gboing.sh || exit 1

# Globals
typeset -i testSetId
typeset -i minQsortVarId
typeset -i maxQsortVarId
typeset -i minCompilerId
typeset -i maxCompilerId

showUsage() {
    local argv0="$1"


    (($#)) || assert
    shift

    if (($#)); then
        echo "${BAD}ERROR${NORMAL}$: $*"
        echo
    fi

    echo "Usage: ${argv0} -h"
    echo "       ${argv0} [param[param...]] <test_set_name> <test_type>"
    echo
    echo "    -d <filename>  Specify the sqlite database file to use"
    echo "                   (defaults to gboing_tests.db)"
    echo "    -c <filename>  Specify the compiler cfg file to use"
    echo "    -t <filename>  Specify the test-specific cfg file to use"
    echo "    -D \"text\"      Optional description for this test set"
    echo "    -m <bitmask>   Specify test to run by a bitmask with the "
	echo "                   folowing values:"
	echo "                       1 Validation -- test compilation of the test"
	echo "                         program succeeds and that the array is"
	echo "                         correctly sorted."
	echo "                       2 Benchmark -- perform a benchmark test"
	echo "                         against GLIBC's legacy qsort and msort"
	echo "                         algos."
	echo "                       4 Function size -- determine the generated"
	echo "                         function size based upon the parameters."
	echo "                   The default is all tests."
    echo "    -h             Show this usage information"
    echo
    echo "    <test_set_name> A name for this test set"
    echo "    <test_type>     Currently, 'qsort' is the only supported test"
    die
    exit 2
}

runQsort() {
echo
}

main() {
    local testSetName
    local testType
    local compilerConf
	typeset -i testMask=7
    local dbFileName="gboing_tests.db"
    local compilerConf=/dev/null

    while getopts "d:c:t:D:m:h" arg "$@"; do
        case "${arg}" in
            d)
                dbFileName="${OPTARG}"
                ;;
            c)
                compilerConf="${OPTARG}"
                ;;
            t)
                testConf="${OPTARG}"
                ;;
            D)
                desc="${OPTARG}"
                ;;
            m)
                testMask=${OPTARG}
                ;;
            ?|h)
                showUsage "$0"
                ;;
        esac
    done

    test -n "${testConf}" || showUsage "$0" "need test config"
    cat "${testConf}" > /dev/null || die

    shift $((OPTIND-1))
    (($# >= 2)) || showUsage "$0"
    testSetName="$1"
    testType="$2"
    shift 2

    initDb "${dbFileName}"
    typeset -i testSetId

    case "${testType}" in
        qsort)
            printf "\nAdding test set...\n"
            addTestSet "${dbFileName}" ${testMask} "${testSetName}" "${desc}" "" || die
            echo testSetId=$testSetId

            printf "\nAdding test variants...\n"
            addQsortVariants "${dbFileName}" ${testSetId} "${testConf}" || die
            echo minQsortVarId=${minQsortVarId}
            echo maxQsortVarId=${maxQsortVarId}

            printf "\nAdding compilers...\n"
            addCompilers "${dbFileName}" ${testSetId} "${compilerConf}" || die
            echo minCompilerId=${minCompilerId}
            echo maxCompilerId=${maxCompilerId}

            printf "\nBuilding test result table...\n"
            populateResultsTable "${dbFileName}" ${testSetId} || die

            ;;
        *)
            showUsage "$0" "unrecognized test type: '$testType'"
            ;;
    esac

}

main "$@"
