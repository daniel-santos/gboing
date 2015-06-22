#!/bin/bash

typeset -i GBOING_SCRIPTS_SOURCED=1

#set -o functrace
#shopt -s extdebug

GOOD=$'\e[32;01m'
WARN=$'\e[33;01m'
BAD=$'\e[31;01m'
NORMAL=$'\e[0m'
HILITE=$'\e[36;01m'
BRACKET=$'\e[34;01m'
BOLD=$'\e[1m'

warn() {
    echo -e "${WARN}WARNING${NORMAL}: $*" >&2
}

die() {
    set +x
    echo -e "${BAD}ERROR${NORMAL}$(test $# -eq 0 || echo ": $*")\n" >&2
    backtrace 1 >&2
    exit 1
}

assert() {
    set +x
    echo -e "${BAD}Internal Script Error${NORMAL}: $*\n" >&2
    backtrace 1 >&2
    exit 1
}

#############################################################################
# Function    : backtrace
# Arguments   : $1 (optional) nuber of calling frames to skip
# Description : Prints out a cute Bash call stack backtrace.  For it to fully
#               function, you must set -o functrace and shopt -s extdebug
#############################################################################
backtrace() {
    typeset -i i arg=0 frame=0 skip=${1:-0} totalArgs=$#
    local fn

    if false; then
        for ((i = 0; 1 ; ++i)); do
            caller $i > /dev/null || break;
            echo "[$i] argc=$((BASH_ARGC[$((i + 1))])) $(caller $i)"
        done

        echo
        for ((i = 0; 1 ; ++i)); do
            asdf="${BASH_ARGV[$((i))]}"
            ((${#asdf})) || break;
            echo "asdf[$((i))]=${asdf}"
        done

        echo
    fi

    for ((frame = 0; 1; ++frame)); do
        caller ${frame} > /dev/null || break;
        ((argsInFrame = BASH_ARGC[$((frame + 1))]))

        if ((frame >= skip)); then
            echo "${HILITE}[$((frame - skip))]${NORMAL} $(caller ${frame})"

            for ((i = 0; i < argsInFrame; ++i)); do
                ((arg = totalArgs + argsInFrame - i - 1))
                echo "    ${HILITE}\$$((i + 1))${NORMAL}: ${BASH_ARGV[${arg}]}"
            done
        fi
        ((totalArgs += argsInFrame))
    done

    if false; then
        if set -o | egrep 'functrace\s+off' > /dev/null; then
            echo "backtrace will not work without set -o functrace"
        fi
    fi

    if shopt | egrep 'extdebug\s+off' >> /dev/null; then
        echo "To see parameters in all frames, use shopt -s extdebug"
    fi
}


##############################################################################
# Function    : yesNo
# Description : Prompts user for a yes/no response via stdout/stdin.
# Arguments   : $1 (optional) A default value to use if the user hits enter
#                  without specifying y or n. Valid values are "y", "n" or ""
#                  (empty or missing). If not suppliedl, yesNo() will
#                  continually re-prompt the user until a valid response is
#                  received.
#               $2 (optional) Text to prompt the user with (will be followed
#                  by the traditional auto-generated "[y/n]: " prompt. Will be
#                  printed with echo -e, so escape sequences will be evaluated.
# Returns     : Zero for yes, non-zero for no.
##############################################################################
yesNo() {
    typeset -l default="$1"
    typeset -i defaultValue
    shift
    local prompt="$*"
    local choices="y/n"
    local -l response


    # Setup choices & sanity checks
    if ((${#default})); then
    case "${default}" in
        y|yes)
            choices="${GOOD}Y${NORMAL}/n"
            defaultValue=0
            ;;

        n|no)
            choices="y/${BAD}N${NORMAL}"
            defaultValue=1
            ;;

        *)
            assert "yesNo() Takes either \"y\", \"n\" or \"\" as" \
                   "it's 1st parameter.  You supplied \"$default\""
            ;;
    esac
    fi

    # main loop
    while true; do
        # Print any prompt
        ((${#prompt})) && echo -e "${prompt} \c"
        echo -e "[${choices}]: \c"
        read response

        # If they just hit enter and there's a default, take that
        ((${#response} == 0 && ${#default})) && return $defaultValue

        case "${response:0:1}" in
            y) return 0;;
            n) return 1;;
        esac
    done
}

#############################################################################
# Function    : for_each
# Arguments   : $1 variable name
#               $2 value list
#               $3 next command (function or program)
#               [$4-?] any arguments to be passed to next command
# Description :
#############################################################################
for_each() {
    (($# >=3 )) || assert "for_each requires at least 3 arguments"

    local _for_each_var=$1
    local _for_each_list="$2"
    local _for_each_i
    shift 2

    for _for_each_i in ${_for_each_list}; do
        eval export ${_for_each_var}="\"${_for_each_i}\""
        "$@" || die "Failed to call next function in for_each chain: $*"
    done
}


joinVars() {
    (($# == 3)) || assert
    local first="$1"
    local delim="$2"
    local second="$3"

    if ((! ${#first})); then
        printf "%s" "${second}"
        return 0
    fi

    if ((! ${#second})); then
        printf "%s" "${first}"
        return 0
    fi

    # Remove redundant delimiters
    if (( ${#delim} )); then
        typeset -i oldlen

        # Remove all trailing delimiters
        for ((oldlen = -1; oldlen != ${#first};)); do
            oldlen=${#first}
            first="${first##${delim}}"
        done

        # Remove all leading delimiters
        for ((oldlen = -1; oldlen != ${#second};)); do
            oldlen=${#second}
            second="${second%%${delim}}"
        done
    fi

    printf "%s" "${first}${delim}${second}"
}

printVars() {
    local var_name
    local var_mode
    local var_value
    local cur_value
    local var_delim
    local export_value
    local vars
    local var_names


    printf "for_each_var_all_names=\"%s\" %c\n" "$(
        for var_name in ${for_each_var_all_names} ${for_each_var_names[*]}; do
            printf "${var_name} "
        done | sort -u | tr '\n' ' ' | sed 's/ *$//g;'
    )" '\\'

    for ((vi = 0; vi < num_vars; ++vi)); do
        var_name=${for_each_var_names[$vi]}
        var_mode=${for_each_var_mode[${var_name}]}
        var_delim=${for_each_var_join_delim[${var_name}]}
        var_value=${for_each_vars[$1,${var_name}]}

        eval eval cur_val="\"\"\$\{${var_name}\}\"\""

        if ((${#var_value} || ${#cur_val})); then
#            printf "***********            ${var_name}=${var_value}\n" >&2
            printf "%s=\"" "${var_name}"

            case ${var_mode} in
            unset|set)
                if ((${#var_value})); then
                    printf "%s" "${var_value}"
                else
                    printf "%s" "${cur_val}"
                fi
                ;;

               append)
                joinVars "${cur_val}" "${var_delim}" "${var_value}"
                ;;

              prepend)
                joinVars "${var_value}" "${var_delim}" "${cur_val}"
                ;;
            esac

            printf "\" %c\n" '\\'
        fi
    done
}

printVarsFromList() {
    local var_names="$1"
    local var_name

    (($# == 1)) || assert

    for var_name in ${var_names}; do
        eval eval val="\"\"\$\{${var_name}\}\"\""
        ((${#val})) || continue
        printf "%s=\"%s\"\n" "${var_name}" "${val}"
    done
}

escapeSqlStringOrNull() {
    (($# == 1)) || assert

    if  [[ -z "$1" ]]; then
        echo "null"
    else
        echo "'$(echo "$1" | sed "s/'/&&/g;")'"
    fi
}

initDb() {
    typeset -i num_tables=0
    local dbFileName="$1"
    (($# == 1)) || assert "Usage: initDb <filename>"
    test -x "${SQLITE}" || assert "sqlite not found"

    # If the database
    if [[ -f "${dbFileName}" ]]; then
        num_tables=$("${SQLITE}" "${dbFileName}" "
            select count(name)
            from sqlite_master
            where type='table' and name in(
                'Statuses', 'Types', 'TestSets', 'Compilers',
                'QsortVariants', 'QsortResults');
        ")
    fi

    if ((! num_tables)); then
        echo "Initializing database file '${dbFileName}'..."
        echo
        "${SQLITE}" "${dbFileName}" < "${GBOING_SCRIPTS_DIR}/tests.ddl" ||
            die "failed to create or init database ${dbFileName}"
    fi
}

doSql() {
#    (($# == 1)) || assert
    ((${#SQLITE})) || assert
    ((${#dbFileName})) || assert
    local sql="$1"
    typeset -i ret

    if ((${#sql})); then
        "${SQLITE}" "${dbFileName}" "${sql}"
    else
        "${SQLITE}" "${dbFileName}"
    fi

    ret=$?
    if ((ret)); then
        echo "Error executing sql statement:" >&2
        echo "${sql}" >&2
        return ${ret}
    fi
}


addTestSet() {
    local dbFileName="$1"
    typeset -i type=$2
    local name=$(escapeSqlStringOrNull "$3")
    local desc=$(escapeSqlStringOrNull "$4")
    local params=$(escapeSqlStringOrNull "$5")
    (($# == 5)) || assert

    doSql "
        insert into TestSets (type, name, desc, params, status)
        values (${type}, ${name}, ${desc}, ${params}, 1);
    " || die

    testSetId=$(doSql "select max(id) from TestSets;") || die
}


addCompilers() {
    declare -a confs
    declare -i num_confs
    declare -i testSetId

    (($# >= 3)) || assert
    dbFileName="$1"
    testSetId=$2
    shift 2

    while (($#)); do
        test -r "$1" || die "Cannot find or read file $1"
        confs[${#confs[*]}]="$1"
        shift
    done

    conf_args=$(
        for ((i = 0; i < ${#confs[*]}; ++i)); do
            printf -- "-c %s" "${confs[i]}"
        done
    )

    minCompilerId=$(doSql "select max(id) from Compilers;") || die
    ((++minCompilerId))

    for_each_compiler ${conf_args} add_compiler "${testSetId}" | doSql || die

    maxCompilerId=$(doSql "select max(id) from Compilers;") || die
}


qsortInsertVariant() {
    typeset -i n=$((data_size / size))
    typeset -i max_size_bits=0

    # Skip tests with too few elements
    if ((n < 16)); then
        return;
    fi

    # Skip size / alignment combinations that don't work
    if ((size < align || (size % align))); then
        return;
    fi

    if ((align & (align - 1))); then
        echo "WARNING: Alignment not a power of two: ${align} (omitting)" >&2
        return;
    fi

    if ((min_size_bits)); then
        typeset -i i

       for ((i = n; i; i >>= 1)); do
            max_size_bits+=1
        done
    fi

    printf "%u,%u,%u,%u,%u,%u,%u,%q,%u,%u,%u,%u,%u\n" \
           $((nextVariantId++)) ${testSetId} ${data_size} ${key_sign} ${n} \
           ${size} ${align} ${less_fn} ${outline_copy} ${outline_swap} \
           ${supply_buffer} ${max_size_bits} ${max_thresh}
}

qsortInsertVariants() {
    test -x "${SQLITE}" || assert "sqlite not found"
    local tmp_file=/tmp/qsort.$$.tmp

    for_each data_size      "${qsort_data_sizes}"    \
    for_each size           "${qsort_elem_sizes}"    \
    for_each align          "${qsort_alignments}"    \
    for_each less_fn        "${qsort_less_fns}"      \
    for_each key_sign       "${qsort_key_signs}"     \
    for_each outline_copy   "${qsort_outline_copy}"  \
    for_each outline_swap   "${qsort_outline_swap}"  \
    for_each supply_buffer  "${qsort_supply_buffer}" \
    for_each min_size_bits  "${qsort_min_size_bits}" \
    for_each max_thresh     "${qsort_max_thresh}"    \
    qsortInsertVariant > "${tmp_file}"

    cat << asdf | doSql || die "sqlite import failed"
.separator ","
.bail on
.import "${tmp_file}" QsortVariants
asdf
    rm "${tmp_file}"
}

addQsortVariants() {
    (($# == 3)) || assert
    local dbFileName="$1"
    typeset -i testSetId="$2"
    local configFile="$3"

    [[ -r "${configFile}" ]] || die "config file ${configFile} doesn't exist or not readable"
    . "${configFile}"

    ((${#qsort_data_sizes}))    || die "qsort_data_sizes not defined"
    ((${#qsort_elem_sizes}))    || die "qsort_elem_sizes not defined"
    ((${#qsort_alignments}))    || die "qsort_alignments not defined"
    ((${#qsort_less_fns}))      || die "qsort_less_fns not defined"
    ((${#qsort_key_signs}))     || die "qsort_key_signs not defined"
    ((${#qsort_outline_copy}))  || die "qsort_outline_copy not defined"
    ((${#qsort_outline_swap}))  || die "qsort_outline_swap not defined"
    ((${#qsort_supply_buffer})) || die "qsort_supply_buffer not defined"
    ((${#qsort_min_size_bits})) || die "qsort_min_size_bits not defined"
    ((${#qsort_max_thresh}))    || die "qsort_max_thresh not defined"

    ((${#CC})) || export CC=cc

    initDb "${dbFileName}"

    minQsortVarId=$(doSql "select max(id) from QsortVariants;") || die
    ((++minQsortVarId))

    nextVariantId=minQsortVarId

    qsortInsertVariants || die

    maxQsortVarId=$(doSql "select max(id) from QsortVariants;") || die
}

populateResultsTable() {
    (($# == 2)) || assert
    local dbFileName="$1"
    typeset -i testSetId="$2"

    doSql "
        insert into QsortResults
        (
            id,
            testSetId,
            compilerId,
            variantId,
            status,
            validated,
            fnSize,
            startTime,
            endTime,
            count0,
            count1,
            count2,
            time0,
            time1,
            time2,
            ips0,
            ips1,
            ips2
        )
        select
            null as id,
            ${testSetId} as testSetId,
            c.id as compilerId,
            v.id as variantId,
            1 as status,
            0 as validated,
            null as fnSize,
            null as startTime,
            null as endTime,
            null as count0,
            null as count1,
            null as count2,
            null as time0,
            null as time1,
            null as time2,
            null as ips0,
            null as ips1,
            null as ips2
        from
            Compilers as c,QsortVariants as v on c.testSetId = v.testSetId
        where
            c.testSetId = ${testSetId}
        order by v.dataSize, v.id;
    "
}

getTestSetByName() {
    (($# == 2)) || assert
    local dbFileName="$1"
    local testSetName="$2"

    initDb "${dbFileName}"

    printf "select id from TestSets where name=%s;" \
           "$(escapeSqlStringOrNull "${testSetName}")" |
    doSql
}

qsortGetNextTest() {
    (($# == 2)) || assert
    local dbFileName="$1"
    typeset -i testSetId="$2"

    doSql "
        select min(id)
        from QsortResults
        where testSetId=${testSetId} and status=1;
    "
}

qsortGetEnv() {
    (($# == 2)) || assert
    local dbFileName="$1"
    local -i testId="$2"

    doSql "
        select
            printf('local extra_CPPFLAGS=\"\
-DELEM_SIZE=%u -DALIGN_SIZE=%u -DKEY_SIGN=%s -DLESS_FN=%s -DOUTLINE_COPY=%u \
-DOUTLINE_SWAP=%u -DSUPPLY_BUFFER=%u -DMAX_SIZE_BITS=%u -DMAX_THRESH=%u\"
local data_size=%u
local size=%u
local align=%u
local less_fn=%s
local key_sign=%u
local outline_copy=%u
local outline_swap=%u
local supply_buffer=%u
local min_size_bits=%u
local max_thresh=%u
%s',
                v.elemSize, v.align,
                case when v.signedKey then 'int' else 'uint' end,
                v.less_fn, v.outlineCopy, v.outlineSwap, v.supplyBuffer,
                v.maxSizeBits, v.maxThresh,
                v.dataSize,
                v.elemSize,
                v.align,
                v.less_fn,
                v.signedKey,
                v.outlineCopy,
                v.outlineSwap,
                v.supplyBuffer,
                v.maxSizeBits,
                v.maxThresh,
                c.env)
        from
            (QsortResults as r inner join QsortVariants as v
                on r.variantId = v.id)
            inner join Compilers as c on r.compilerId = c.id
        where r.Id=${testId} and status=1;
    "
}

dumpArgs() {
    echo "0=$0"
    for ((i = 0; $# > 0; ++i)); do
        echo "$i=$1"
        shift
    done
}

qsortRunTest() {
    (($# == 4)) || assert
    local dbFileName="$1"
    local testId="$2"
    typeset -i testMask=$3
    local env="$4"

    local qsortTestArgs
    local output=""
    local benchmarkFields
    local targets=""
    typeset -i validation=0
    typeset -i validationFailed=0

#    echo "testId=${testId}"

    . "${GBOING_SCRIPTS_DIR}/compiler-base.conf" || die

    # Clear make-related variables
    unset ${for_each_var_names[*]}

    # source the environment
    eval "${env}"

    # Append extra_CPPFLAGS (skip space if CPPFLAGS isn't set)
    CPPFLAGS="${CPPFLAGS}${CPPFLAGS+ }${extra_CPPFLAGS}"

    # Export make-related variables
    export ${for_each_var_names[*]}
    # ...except for this one
    unset extra_CPPFLAGS

    doSql "
        update QsortResults
        set
            status=2,
            startTime=datetime('now')
        where id=${testId};
    " || die

    if ((testMask & 4)); then
        targets="qsort-instantiate.o"
    fi

    if ((testMask & 3)); then
        targets="${targets} qsort"
    fi

    # Create build directory and and copy makefile
    mkdir -p "${BUILD_DIR}"
    cp "${GBOING_SCRIPTS_DIR}/Makefile" "${BUILD_DIR}" || die

    make -C "${BUILD_DIR}" clean || die
    make -j${NUM_JOBS} -C "${BUILD_DIR}" ${targets} || die

    if ((testMask & 4)); then
        ${BINUTILS_PREFIX}strip -s "${BUILD_DIR}/qsort-instantiate.o" || die

        fn_size=$(
            printf "%u\n" $(
                ${BINUTILS_PREFIX}objdump -x "${BUILD_DIR}/qsort-instantiate.o" |
                grep '\.text' |
                grep -v '\.text\.unlikely' |
                awk '{print "0x" $3}'
            ) || exit 2
        ) || die

        echo "Function size: $fn_size"
        echo
        echo

        doSql "
            update QsortResults
            set
                fnSize=${fn_size}
            where id=${testId};
        " || die
    fi

    if ((testMask & 3)); then

        if ((testMask & 2)); then
            qsortTestArgs="--max-time 1"
        else
            qsortTestArgs="--max-iterations 1"
        fi

        echo "${BUILD_DIR}/qsort" -v ${qsortTestArgs} --data-size ${data_size}
        output=$(
            "${BUILD_DIR}/qsort" -v ${qsortTestArgs} --data-size ${data_size}
        )

        if (($?)); then
            validationFailed=1
        else
            benchmarkFields=$(
                echo "${output}" |
                awk '{print "count0=" $1 ", time0=" $2 ", ips0=(" $1 "/" $2 "), " \
                            "count1=" $3 ", time1=" $4 ", ips1=(" $3 "/" $4 "), " \
                            "count2=" $5 ", time2=" $6 ", ips2=(" $5 "/" $6 ")"   \
                }'
            )
        fi
        echo "$output"

        validation=1
    fi

    # If we just ran validation but not benchmarks then clear output
    ((testMask & 2)) || unset output

    doSql "
        update QsortResults
        set
            status=$((validationFailed ? 4 : 5)),
            validated=${validation},
            endTime=datetime('now')
            ${benchmarkFields+,} ${benchmarkFields}
        where id=${testId};
    " || die

    unset ${for_each_var_names[*]}
}

