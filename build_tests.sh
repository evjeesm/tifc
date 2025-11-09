#!/bin/sh

CFLAGS='-std=gnu18 -Wall -Wextra -Wno-override-init -Werror'
gcc -g -O0 $CFLAGS input_test.c input.c -L deps -I deps -I ui -I input -lcircbuf_static -lvector_static -lhashmap_static -o input_test

gcc -g -O0 $CFLAGS display_test.c display.c input.c -L deps -I deps -I ui -I display -lcircbuf_static -lvector_static -lhashmap_static -o display_test
