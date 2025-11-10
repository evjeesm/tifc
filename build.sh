#!/bin/sh
# set -xe

ROOT_DIR="$(dirname $0)"
CC="gcc"

BUILD_DIR="./build"
STAMP_DIR="${BUILD_DIR}/.stamps"

list() { echo $@ ;}

SUBDIRS=$(list "
    core
    core/display
    core/input
    core/logger
    core/ui

    client
")

CFLAGS=$(list "
    -g
    -O0
    -std=gnu18
    -Wall
    -Wextra
    -Wno-override-init
    -Werror
")

CPPFLAGS=$(list "
    -I.
    -Ldeps
    -Ideps
")

SOURCES=$(list "
    tifc.c
    canvas.c
    frame.c
")

LIBS=$(list "
    -lcircbuf_static
    -lhashmap_static
    -lsparse_static
    -ldynarr_static
    -lvector_static
    -lm
")

init_subdirs() {
    for dir in ${SUBDIRS}; do
        CPPFLAGS="${CPPFLAGS} -I${dir}"
        mkdir -p ${BUILD_DIR}/${dir}
        mkdir -p ${STAMP_DIR}/${dir}
    done
}


# Compare first 'ts' vs list of 'ts' that comes after.
# Return: '1' if any of the 'ts' in a list is newer than the first 'ts',
#         otherwise '0'
is_newer_source() {
    [ ${#} -lt 2 ] \
        && echo "! is_newer_source() expects (object, dep[, ...])" >&2 \
        && return 0

    local object=${1}; shift
    for dep in ${@}; do
        [ ${object} -ot ${dep} ] && return 1
    done
    return 0
}

check() {
    [ ${#} -eq 0 ] \
        && echo "! check() expects an object" >&2 \
        && exit 1

    local obj=${1}
    local obj_path="${BUILD_DIR}/${1}"
    [ ! -e ${obj_path} ] \
        && echo "No such object '${obj_path}'" >&2 \
        && return 1

    local sum="${STAMP_DIR}/${obj}.sha1"
    { sha1sum -c ${sum} >&2 && return 0 || return 1 ;}
}

build_objects() {
    # Build objects from sources
    local compiled=0
    local objects=""
    for src in $@; do
        # source -> object
        local obj=$(echo ${src} | sed 's/\.c/\.o/g')
        local obj_path="${BUILD_DIR}/${obj}"
        local deps=""
        deps=$(collect_dependencies ${src} "${deps}")
        local sum="${STAMP_DIR}/${obj}.sha1"

        # append to object list
        objects="${objects} ${obj_path}"

        if [ -e ${obj_path} ]; then
            # is newer source
            [ 0 = $( is_newer_source ${obj_path} ${deps}; echo $? ) ] && continue
        fi

        # compile
        local cmd="${CC} ${CPPFLAGS} ${CFLAGS} -c ${src} -o ${obj_path}"
        ${cmd}
        local status=$?
        : echo "CC STATUS: $status" >&2

        [ "$status" -ne '0' ] && echo ${compiled} && { exit "$status" ;} # failure

        # success
        echo "${cmd}" >&2
        sha1sum ${deps} > ${sum} # recalculate sum

        compiled=$(( $compiled + 1 ))
    done
    echo ${compiled}
}

gen_compile_commands() {
    # Build objects from sources
    for dir in ${ROOT_DIR} ${SUBDIRS}; do
        for src in ${dir}/*.c; do
            # source -> object
            local obj=$(echo ${src} | sed 's/\.c/\.o/g')
            local obj_path="${BUILD_DIR}/${obj}"
            # local deps=""
            # deps=$( collect_dependencies ${src} ${deps})

            # compile
            local cmd="${CC} ${CPPFLAGS} ${CFLAGS} -c ${src} -o ${obj_path}"
            echo ${cmd}
        done
    done
}

collect_tests() {
    # collect tests
    local tests=""
    for dir in ${SUBDIRS}; do
        for _test_ in ${dir}/*_test.c; do
            if [ -e ${_test_} ]; then tests="${tests} ${_test_}"; fi
        done
    done
    echo "collected :${tests}" >&2
    echo ${tests}
}

collect_dependencies() {
    [ $# = 0 ] && { echo "! collect_dependencies() expects source. " >&2 && exit 1 ;}

    [ $# = 1 ] && { echo "! collect_dependencies() expects dependency_list " >&2 && exit 1 ;}

    local list
    list=$2

    # get list of header separated with space
    local dependencies
    dependencies=$( ${CC} ${CPPFLAGS} -MM $1 | tr -d '\n' \
        | sed 's/ [\]//g;' \
        | sed 's/.*: //g' \
        | xargs -n1 | sort -u | xargs )

    local collected
    for dep in $dependencies; do
        collected="false"
        for list_dep in $list; do
            if [ "$dep" = "$list_dep" ]; then
                # echo "$dep already collected" >&2
                collected="true"
                break
            fi
        done
        if [ "$collected" = "false" ]; then
            list="$list $dep"
            # echo "Collecting $dep" >&2
            list=$(collect_dependencies ${dep} "$list")
        fi
    done
    echo ${list}
}

h2c() {
    local dep_sources=""
    for header in "$@"; do
        local src=$(echo ${header} | sed 's/\.h/\.c/')
        [ -e ${src} -a ${source} != ${src} ] && dep_sources="${dep_sources} ${src}"
    done
    echo ${dep_sources}
}

# Function takes executable source file path
# and returns a compile command
build_executable() {
    [ $# -eq 0 ] && { echo "! build_executable() expects source. " >&2 && exit 1 ;}
    local source="$1"
    local target=$( echo "${source}" | sed 's/\.c//')
    echo "Target: $target" >&2

    local deps=""
    deps=$( collect_dependencies ${source} "${deps}")
    local sources="${source} $( h2c ${deps} )"
    local objects=$(echo ${sources} | sed "s/\.c/\.o/g; s@\([./a-zA-Z0-9~_$]\+\)@${BUILD_DIR}/\1@g")
    local sum="${STAMP_DIR}/${target}.sha1"

    local compiled;
    local status;

    compiled=$( build_objects ${sources} )
    status=$?
    : echo "COMPILE STATUS = $status" >&2

    [ "$status" != 0 ] && { return "$status" ;}

    echo "Compiled: $compiled" >&2
    # is newer source
    if [ ${compiled} -eq 0 ] \
    && [ -e ${BUILD_DIR}/${target} ] \
    && [ 0 = $( is_newer_source ${BUILD_DIR}/${target} ${deps}; echo $? ) ]
    then
        echo "Nothing to be done for target '${target}'" >&2
        return 0
    fi

    truncate -s 0 ${sum}
    local obj_sums=$( echo "$objects" \
        | sed "s@${BUILD_DIR}@${STAMP_DIR}@g" \
        | sed "s@\s\|\$@.sha1 @g" )
    sort -m -u -k2 ${obj_sums} -o ${sum}

    # compile
    local cmd="${CC} ${CPPFLAGS} ${CFLAGS} ${objects} -o ${BUILD_DIR}/${target} ${LIBS}"
    ${cmd} || return $? # return on failure
    echo ${cmd} >&2

    return 0
}

help() {
    local compile_desc="compile [<target>]      - compile specific target. default target: 'tifc'"
    local check_desc="check [<target|object>] - check sum of the dependencies."
    local clean_desc="clean                   - Remove all build artifacts."
    local init_clangd_desc="init_clangd             - Generates 'compile_commands.json' file for clangd. "
    local help_desc="help                    - show this help menu."
    case "$1" in
        compile)
            echo "${compile_desc}"
            echo "Available targets:\n\ttifc\n\ttests"
        ;;
        check)
            echo "\t${check_desc}"
        ;;
        clean)
            echo "\t${clean_desc}"
        ;;
        init_clangd)
            echo "\t${init_clangd_desc}"
        ;;
        help)
            echo "\t${help_desc}"
        ;;
        *)
            echo "Available sub-commands:"
            echo "\t${compile_desc}"
            echo "\t${clean_desc}"
            echo "\t${init_clangd_desc}"
            echo "\t${help_desc}"
        ;;
    esac
}

main() {
    local sub_cmd=${1:-'help'}
    case "${sub_cmd}" in
        compile)
            local target=${2:-'tifc'}
            case "$target" in
                tifc)
                    { build_executable 'core/tifc.c' ;}
                    [ $? != 0 ] && exit $?
                ;;
                tests)
                    local tests=$( collect_tests )
                    for _test_ in ${tests}; do
                        { build_executable ${_test_} ;}
                    done
                ;;
                *) echo "ERROR : Wrong compile target '$2'" >&2
                ;;
            esac
        ;;
        check)
            local target=${2}
            check ${target}
        ;;
        clean)
            local cmd="rm -rf ${BUILD_DIR}"
            echo "${cmd}"
            $($cmd)
        ;;
        init_clangd)
            gen_compile_commands \
                | grep -wE "${CC}" \
                | grep -w '\-c' \
                | jq -nR '[inputs|{directory:env.PWD, command:., file: match(" [^ ]+$").string[1:]}]' \
                > compile_commands.json
        ;;
        help)
            help $2
        ;;
        *)
            echo "ERROR : Wrong sub-command '${sub_cmd}'" >&2
            help ${sub_cmd} >&2
            exit 1
        ;;
    esac
}

{
    cd ${ROOT_DIR}      # enter project dir
    init_subdirs
    main $@
    cd - > /dev/null    # leave project dir
}

