#!/bin/sh
#set -xe

ROOT_DIR="$(dirname $0)"
CC="gcc"

BUILD_DIR="./build"
STAMP_DIR="${BUILD_DIR}/.stamps"

list() { echo $@ ;}

SUBDIRS=$(list "
    display
    ui
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
    input.c
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
        mkdir -p ${BUILD_DIR}/$(basename $dir)
        mkdir -p ${STAMP_DIR}/$(basename $dir)
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
    local objects=""
    for src in $@; do
        # source -> object
        local obj=$(echo ${src} | sed 's/\.c/\.o/g')
        local obj_path="${BUILD_DIR}/${obj}"
        local deps=$( collect_dependencies ${src} )
        local sum="${STAMP_DIR}/${obj}.sha1"

        # append to object list
        objects="${objects} ${obj_path}"

        if [ -e ${obj_path} ]; then
            # is newer source
            [ 0 = $( is_newer_source ${obj_path} ${deps}; echo $? ) ] && continue
        fi

        # compile
        local cmd="${CC} ${CPPFLAGS} ${CFLAGS} -c ${src} -o ${obj_path}"
        ${cmd} || return $? # return on failure
        echo "${cmd}" >&2
        sha1sum ${deps} > ${sum} # recalculate sum
    done
    echo ${objects}
}

collect_tests() {
    # collect tests
    local tests=""
    for dir in ${SUBDIRS}; do
        for _test_ in ${dir}/*_test.c; do
            tests="${tests} ${_test_}"
        done
    done
    echo ${tests}
}

collect_dependencies() {
    [ $# = 0 ] && { echo "! collect_dependencies() expects source. " >&2 && exit 1 ;}

    # get list of header separated with space
    local dependencies=$( ${CC} ${CPPFLAGS} -MM $1 | tr -d '\n' \
        | sed 's/ [\]//g;' \
        | sed 's/.*: //g' \
        | xargs -n1 | sort -u | xargs )

    echo ${dependencies}
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

    local deps=$( collect_dependencies ${source} )
    local sources="${source} $( h2c ${deps} )"
    local sum="${STAMP_DIR}/${target}.sha1"

    local objects=$( build_objects ${sources} )
    [ $? != 0 ] && return $?

    # is newer source
    if [ -e ${BUILD_DIR}/${target} ] \
    && [ 0 = $( is_newer_source ${BUILD_DIR}/${target} ${deps}; echo $? ) ]; then
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
        help)
            echo "\t${help_desc}"
        ;;
        *)
            echo "Available sub-commands:"
            echo "\t${compile_desc}"
            echo "\t${clean_desc}"
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
                    { build_executable 'tifc.c' ;}
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
            { check ${target} ;}
        ;;
        clean)
            local cmd="rm -rf ${BUILD_DIR}"
            echo "${cmd}"
            $($cmd)
        ;;
        help)
            { help $2 ;}
        ;;
        *)
            echo "ERROR : Wrong sub-command '${sub_cmd}'" >&2
            { help ${sub_cmd} >&2 ;}
        ;;
    esac
}

{
    cd ${ROOT_DIR}      # enter project dir
    init_subdirs
    main $@
    cd - > /dev/null    # leave project dir
}

