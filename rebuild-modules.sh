#!/bin/sh

THREADS=8
CFLAGS="-g -O0 -Wall -Wextra -Werror -Wno-override-init -Wno-unused-function"

HELP='List of sub-commands:\n
\t update      \t  Recusively init/update submodules.\n
\t remove-all  \t  Remove submodules and clear folders.\n
\t fresh     \t\t  Configure and generate build files (prior `update` invokation required).\n
\t config      \t  Configure and clean compiled artifacts (maybe usefull if `CFLAGS` changed).\n
\t remake      \t  Clean all build files and then make.\n
\t make      \t\t  Just make. (if source code of submodule changed)\n
'

[ $# = 0 ] && { echo $HELP; exit 0; }
[ $# -ge 1 ] && {
    case $1 in
        update)
            git submodule update --init --recursive && git submodule update --init --remote
            [ $? -ne 0 ] && exit $? || exit 0
        ;;
        remove-all)
            echo Removing submodules!
            git submodule deinit --force --all
            [ $? -ne 0 ] && exit $? || exit 0
        ;;
        fresh)
            echo Fresh ...
            git submodule foreach "make -j$THREADS distclean; ./autogen.sh && ./configure"
            [ $? -ne 0 ] && exit $? || exit 0
        ;;
        config)
            echo Configuring ...
            git submodule foreach "./configure CFLAGS=\"$CFLAGS\" && make -j$THREADS clean"
            [ $? -ne 0 ] && exit $? || exit 0
        ;;
        remake)
            echo Clean Remaking ...
            git submodule foreach "make -j$THREADS clean; make -j$THREADS"
            [ $? -ne 0 ] && exit $? || exit 0
        ;;
        make)
            echo Making ...
            git submodule foreach "make -j$THREADS"
            [ $? -ne 0 ] && exit $? || exit 0
        ;;
        *)
            echo Unknown sub-command!
            exit 1
        ;;
    esac
}
